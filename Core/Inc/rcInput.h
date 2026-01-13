#ifndef RC_INPUT_H
#define RC_INPUT_H

#include "stm32g4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define RC_INPUT_CHANNEL_COUNT 4U

typedef struct
{
  uint16_t pulseWidthUs[RC_INPUT_CHANNEL_COUNT];
  bool signalValid[RC_INPUT_CHANNEL_COUNT];
  uint32_t lastUpdateMs[RC_INPUT_CHANNEL_COUNT];
} RcInputState;

/***************** rcInputInit ************************************************
 * params: TIM_HandleTypeDef *tim2Handle, TIM_HandleTypeDef *tim3Handle
 * return: void
 * Description:
 * Initializes the RC input capture module for TIM2 and TIM3 according to the
 * fixed channel mapping used on the Nucleo-G474 setup.
 ******************************************************************************/
void rcInputInit(TIM_HandleTypeDef *tim2Handle, TIM_HandleTypeDef *tim3Handle);

/***************** rcInputStart ***********************************************
 * params: none
 * return: void
 * Description:
 * Starts input capture on all configured RC input channels (TIM2 CH2+CH3,
 * TIM3 CH1+CH2).
 ******************************************************************************/
void rcInputStart(void);

/***************** rcInputProcessCapture **************************************
 * params: TIM_HandleTypeDef *timerHandle
 * return: void
 * Description:
 * Processes timer input capture callbacks to measure pulse widths.
 * Call this from HAL_TIM_IC_CaptureCallback() for BOTH TIM2 and TIM3.
 ******************************************************************************/
void rcInputProcessCapture(TIM_HandleTypeDef *timerHandle);

/***************** rcInputUpdate **********************************************
 * params: uint32_t nowMs
 * return: void
 * Description:
 * Updates signal validity based on the failsafe timeout.
 ******************************************************************************/
void rcInputUpdate(uint32_t nowMs);

/***************** rcInputGetState ********************************************
 * params: none
 * return: RcInputState
 * Description:
 * Returns the latest RC input state.
 ******************************************************************************/
RcInputState rcInputGetState(void);

#endif
