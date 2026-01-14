#include "engineModel.h"
#include "boardConfig.h"
#include "soundData.h"

static EngineModelState engineState;
static uint16_t lastThrottle;
static uint32_t wastegateMillis;
static bool wastegateTrigger;

static uint16_t engineModelMap(uint16_t input, uint16_t inMin, uint16_t inMax, uint16_t outMin, uint16_t outMax)
{
  if (input <= inMin)
  {
    return outMin;
  }
  if (input >= inMax)
  {
    return outMax;
  }

  uint32_t numerator = (uint32_t)(input - inMin) * (uint32_t)(outMax - outMin);
  uint32_t denom = (uint32_t)(inMax - inMin);
  return (uint16_t)(outMin + (numerator / denom));
}

static uint16_t engineModelComputeThrottle(uint16_t pulseWidthUs)
{
  if (pulseWidthUs < RC_PULSE_MIN_US || pulseWidthUs > RC_PULSE_MAX_US)
  {
    return 0U;
  }

  if (pulseWidthUs > (RC_PULSE_NEUTRAL_US + RC_PULSE_DEADBAND_US))
  {
    return engineModelMap(pulseWidthUs, RC_PULSE_NEUTRAL_US + RC_PULSE_DEADBAND_US, RC_PULSE_MAX_US, 0U, 500U);
  }

  if (pulseWidthUs < (RC_PULSE_NEUTRAL_US - RC_PULSE_DEADBAND_US))
  {
    return engineModelMap(pulseWidthUs, RC_PULSE_NEUTRAL_US - RC_PULSE_DEADBAND_US, RC_PULSE_MIN_US, 0U, 500U);
  }

  return 0U;
}

static void engineModelUpdateThrottleFade(void)
{
  if (engineState.currentThrottleFaded < engineState.currentThrottle && engineState.currentThrottleFaded < 499U)
  {
    engineState.currentThrottleFaded += 2U;
  }
  if (engineState.currentThrottleFaded > engineState.currentThrottle && engineState.currentThrottleFaded > 1U)
  {
    engineState.currentThrottleFaded -= 2U;
  }
}

static void engineModelUpdateVolumes(void)
{
  engineState.throttleDependentVolume = engineModelMap(
    engineState.currentThrottleFaded,
    0U,
    500U,
    (uint16_t)engineIdleVolumePercentage,
    (uint16_t)fullThrottleVolumePercentage);

  engineState.throttleDependentRevVolume = engineModelMap(
    engineState.currentThrottleFaded,
    0U,
    500U,
    (uint16_t)engineRevVolumePercentage,
    (uint16_t)fullThrottleVolumePercentage);

  if (engineState.currentThrottleFaded > (uint16_t)dieselKnockStartPoint)
  {
    engineState.throttleDependentKnockVolume = engineModelMap(
      engineState.currentThrottleFaded,
      (uint16_t)dieselKnockStartPoint,
      500U,
      (uint16_t)dieselKnockIdleVolumePercentage,
      100U);
  }
  else
  {
    if (engineState.throttleDependentKnockVolume > (uint16_t)dieselKnockIdleVolumePercentage)
    {
      engineState.throttleDependentKnockVolume--;
    }
    else
    {
      engineState.throttleDependentKnockVolume = (uint16_t)dieselKnockIdleVolumePercentage;
    }
  }

  engineState.throttleDependentTurboVolume = engineModelMap(
    engineState.currentRpm,
    0U,
    500U,
    (uint16_t)turboIdleVolumePercentage,
    100U);

  if (engineState.currentRpm > (uint16_t)fanStartPoint)
  {
    engineState.throttleDependentFanVolume = engineModelMap(
      engineState.currentRpm,
      (uint16_t)fanStartPoint,
      500U,
      (uint16_t)fanIdleVolumePercentage,
      100U);
  }
  else
  {
    engineState.throttleDependentFanVolume = (uint16_t)fanIdleVolumePercentage;
  }

  if (engineState.currentRpm > (uint16_t)chargerStartPoint)
  {
    engineState.throttleDependentChargerVolume = engineModelMap(
      engineState.currentThrottleFaded,
      (uint16_t)chargerStartPoint,
      500U,
      (uint16_t)chargerIdleVolumePercentage,
      100U);
  }
  else
  {
    engineState.throttleDependentChargerVolume = (uint16_t)chargerIdleVolumePercentage;
  }

  engineState.rpmDependentWastegateVolume = engineModelMap(
    engineState.currentRpm,
    0U,
    500U,
    (uint16_t)wastegateIdleVolumePercentage,
    100U);
}

static void engineModelUpdateRpm(void)
{
  uint16_t targetRpm = engineState.currentThrottle;

  if (targetRpm > 500U)
  {
    targetRpm = 500U;
  }

  if (targetRpm > (uint16_t)(engineState.currentRpm + (uint16_t)acc) && engineState.currentRpm < 500U)
  {
    engineState.currentRpm = (uint16_t)(engineState.currentRpm + (uint16_t)acc);
    if (engineState.currentRpm > 500U)
    {
      engineState.currentRpm = 500U;
    }
  }

  if (targetRpm < engineState.currentRpm)
  {
    if (engineState.currentRpm > (uint16_t)dec)
    {
      engineState.currentRpm = (uint16_t)(engineState.currentRpm - (uint16_t)dec);
    }
    else
    {
      engineState.currentRpm = 0U;
    }
  }

  uint32_t maxSampleInterval = (4000000U / (uint32_t)sampleRate);
  uint32_t minSampleInterval = (maxSampleInterval * 100U) / MAX_RPM_PERCENTAGE;
  uint32_t interval = engineModelMap(engineState.currentRpm, 0U, 500U, (uint16_t)maxSampleInterval, (uint16_t)minSampleInterval);

  engineState.engineSampleIntervalTicks = (uint16_t)interval;
}

static void engineModelUpdateWastegate(uint32_t nowMs)
{
  if ((lastThrottle > engineState.currentThrottle) &&
      (lastThrottle - engineState.currentThrottle > 70U) &&
      (nowMs - wastegateMillis > 1000U))
  {
    wastegateMillis = nowMs;
    wastegateTrigger = true;
  }
  lastThrottle = engineState.currentThrottle;
}

/***************** engineModelInit ********************************************
 * params: none
 * return: void
 * Description:
 * Initializes the engine model state.
 ******************************************************************************/
void engineModelInit(void)
{
  engineState.motorCommand = 0.0f;
  engineState.currentThrottle = 0U;
  engineState.currentThrottleFaded = 0U;
  engineState.currentRpm = 0U;
  engineState.engineSampleIntervalTicks = 0U;
  engineState.throttleDependentVolume = (uint16_t)engineIdleVolumePercentage;
  engineState.throttleDependentRevVolume = (uint16_t)engineRevVolumePercentage;
  engineState.throttleDependentKnockVolume = (uint16_t)dieselKnockIdleVolumePercentage;
  engineState.throttleDependentTurboVolume = (uint16_t)turboIdleVolumePercentage;
  engineState.throttleDependentFanVolume = (uint16_t)fanIdleVolumePercentage;
  engineState.throttleDependentChargerVolume = (uint16_t)chargerIdleVolumePercentage;
  engineState.rpmDependentWastegateVolume = (uint16_t)wastegateIdleVolumePercentage;
  engineState.signalValid = false;
  lastThrottle = 0U;
  wastegateMillis = 0U;
  wastegateTrigger = false;
}

/***************** engineModelUpdate ******************************************
 * params: uint16_t pulseWidthUs, bool signalValid, uint32_t nowMs
 * return: void
 * Description:
 * Updates throttle, RPM, and volume parameters based on RC input.
 ******************************************************************************/
void engineModelUpdate(uint16_t pulseWidthUs, bool signalValid, uint32_t nowMs)
{
  engineState.signalValid = signalValid;

  if (!signalValid)
  {
    engineState.currentThrottle = 0U;
  }
  else
  {
    engineState.currentThrottle = engineModelComputeThrottle(pulseWidthUs);
  }

  engineState.motorCommand = (float)engineState.currentThrottle / 500.0f;

  engineModelUpdateThrottleFade();
  engineModelUpdateRpm();
  engineModelUpdateVolumes();
  engineModelUpdateWastegate(nowMs);
}

/***************** engineModelGetState ****************************************
 * params: none
 * return: EngineModelState
 * Description:
 * Returns the current engine model state.
 ******************************************************************************/
EngineModelState engineModelGetState(void)
{
  return engineState;
}

/***************** engineModelConsumeWastegateTrigger *************************
 * params: none
 * return: bool
 * Description:
 * Returns and clears the wastegate trigger flag.
 ******************************************************************************/
bool engineModelConsumeWastegateTrigger(void)
{
  bool trigger = wastegateTrigger;
  wastegateTrigger = false;
  return trigger;
}
