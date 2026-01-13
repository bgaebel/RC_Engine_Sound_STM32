#include "rcInput.h"
#include "boardConfig.h"

typedef struct
{
  TIM_HandleTypeDef *timerHandle;
  uint32_t timerChannel;
  uint32_t risingEdgeTick;
  bool waitingForFallingEdge;
} RcInputChannelCfg;

static RcInputState rcState;

static TIM_HandleTypeDef *tim2 = NULL;
static TIM_HandleTypeDef *tim3 = NULL;

static RcInputChannelCfg rcCh[RC_INPUT_CHANNEL_COUNT];

static uint32_t rcInputGetActiveChannel(TIM_HandleTypeDef *timerHandle)
{
  if (timerHandle->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    return TIM_CHANNEL_1;
  }
  if (timerHandle->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    return TIM_CHANNEL_2;
  }
  if (timerHandle->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
  {
    return TIM_CHANNEL_3;
  }
  return TIM_CHANNEL_4;
}

static int32_t rcInputFindIndex(TIM_HandleTypeDef *timerHandle, uint32_t activeChannel)
{
  for (uint32_t i = 0U; i < RC_INPUT_CHANNEL_COUNT; i++)
  {
    if ((rcCh[i].timerHandle == timerHandle) && (rcCh[i].timerChannel == activeChannel))
    {
      return (int32_t)i;
    }
  }
  return -1;
}

/***************** rcInputInit ************************************************
 * params: TIM_HandleTypeDef *tim2Handle, TIM_HandleTypeDef *tim3Handle
 * return: void
 * Description:
 * Initializes the RC input capture module for TIM2 and TIM3 according to the
 * fixed channel mapping used on the Nucleo-G474 setup.
 ******************************************************************************/
void rcInputInit(TIM_HandleTypeDef *tim2Handle, TIM_HandleTypeDef *tim3Handle)
{
  tim2 = tim2Handle;
  tim3 = tim3Handle;

  for (uint32_t i = 0U; i < RC_INPUT_CHANNEL_COUNT; i++)
  {
    rcState.pulseWidthUs[i] = RC_PULSE_NEUTRAL_US;
    rcState.signalValid[i] = false;
    rcState.lastUpdateMs[i] = 0U;
  }

  /* Fixed mapping for your wiring:
     CH0: PB3  -> TIM2_CH2
     CH1: PB4  -> TIM3_CH1
     CH2: PB5  -> TIM3_CH2
     CH3: PB10 -> TIM2_CH3
  */
  rcCh[0].timerHandle = tim2;
  rcCh[0].timerChannel = TIM_CHANNEL_2;

  rcCh[1].timerHandle = tim3;
  rcCh[1].timerChannel = TIM_CHANNEL_1;

  rcCh[2].timerHandle = tim3;
  rcCh[2].timerChannel = TIM_CHANNEL_2;

  rcCh[3].timerHandle = tim2;
  rcCh[3].timerChannel = TIM_CHANNEL_3;

  for (uint32_t i = 0U; i < RC_INPUT_CHANNEL_COUNT; i++)
  {
    rcCh[i].risingEdgeTick = 0U;
    rcCh[i].waitingForFallingEdge = false;
  }
}

/***************** rcInputStart ***********************************************
 * params: none
 * return: void
 * Description:
 * Starts input capture on all configured RC input channels (TIM2 CH2+CH3,
 * TIM3 CH1+CH2).
 ******************************************************************************/
void rcInputStart(void)
{
  if ((tim2 == NULL) || (tim3 == NULL))
  {
    return;
  }

  __HAL_TIM_SET_CAPTUREPOLARITY(tim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
  __HAL_TIM_SET_CAPTUREPOLARITY(tim2, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
  __HAL_TIM_SET_CAPTUREPOLARITY(tim3, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
  __HAL_TIM_SET_CAPTUREPOLARITY(tim3, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);

  HAL_TIM_IC_Start_IT(tim2, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(tim2, TIM_CHANNEL_3);
  HAL_TIM_IC_Start_IT(tim3, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(tim3, TIM_CHANNEL_2);
}

/***************** rcInputProcessCapture **************************************
 * params: TIM_HandleTypeDef *timerHandle
 * return: void
 * Description:
 * Processes timer input capture callbacks to measure pulse widths.
 * Call this from HAL_TIM_IC_CaptureCallback() for BOTH TIM2 and TIM3.
 ******************************************************************************/
void rcInputProcessCapture(TIM_HandleTypeDef *timerHandle)
{
  uint32_t activeChannel = rcInputGetActiveChannel(timerHandle);
  int32_t index = rcInputFindIndex(timerHandle, activeChannel);
  if (index < 0)
  {
    return;
  }

  uint32_t captureValue = HAL_TIM_ReadCapturedValue(timerHandle, activeChannel);

  if (rcCh[index].waitingForFallingEdge == false)
  {
    rcCh[index].risingEdgeTick = captureValue;
    rcCh[index].waitingForFallingEdge = true;
    __HAL_TIM_SET_CAPTUREPOLARITY(timerHandle, activeChannel, TIM_INPUTCHANNELPOLARITY_FALLING);
    return;
  }

  uint32_t period = __HAL_TIM_GET_AUTORELOAD(timerHandle);
  uint32_t pulseTicks;

  if (captureValue >= rcCh[index].risingEdgeTick)
  {
    pulseTicks = captureValue - rcCh[index].risingEdgeTick;
  }
  else
  {
    pulseTicks = (period + 1U) - rcCh[index].risingEdgeTick + captureValue;
  }

  /* With PSC=169 at 170MHz, RC_INPUT_TIMER_HZ should be 1,000,000 (1 tick = 1us). */
  uint32_t pulseWidthUs = (pulseTicks * 1000000U) / RC_INPUT_TIMER_HZ;

  if (pulseWidthUs < 500U || pulseWidthUs > 2500U)
  {
    rcState.signalValid[index] = false;
  }
  else
  {
    rcState.pulseWidthUs[index] = (uint16_t)pulseWidthUs;
    rcState.lastUpdateMs[index] = HAL_GetTick();
    rcState.signalValid[index] = true;
  }

  rcCh[index].waitingForFallingEdge = false;
  __HAL_TIM_SET_CAPTUREPOLARITY(timerHandle, activeChannel, TIM_INPUTCHANNELPOLARITY_RISING);
}

/***************** rcInputUpdate **********************************************
 * params: uint32_t nowMs
 * return: void
 * Description:
 * Updates signal validity based on the failsafe timeout.
 ******************************************************************************/
void rcInputUpdate(uint32_t nowMs)
{
  for (uint32_t i = 0U; i < RC_INPUT_CHANNEL_COUNT; i++)
  {
    if ((nowMs - rcState.lastUpdateMs[i]) > RC_FAILSAFE_TIMEOUT_MS)
    {
      rcState.signalValid[i] = false;
    }
  }
}

/***************** rcInputGetState ********************************************
 * params: none
 * return: RcInputState
 * Description:
 * Returns the latest RC input state.
 ******************************************************************************/
RcInputState rcInputGetState(void)
{
  return rcState;
}
