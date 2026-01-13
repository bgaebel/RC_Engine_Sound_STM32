#ifndef ENGINE_MODEL_H
#define ENGINE_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
  float motorCommand;
  uint16_t currentThrottle;
  uint16_t currentThrottleFaded;
  uint16_t currentRpm;
  uint16_t engineSampleIntervalTicks;
  uint16_t throttleDependentVolume;
  uint16_t throttleDependentRevVolume;
  uint16_t throttleDependentKnockVolume;
  uint16_t throttleDependentTurboVolume;
  uint16_t throttleDependentFanVolume;
  uint16_t throttleDependentChargerVolume;
  uint16_t rpmDependentWastegateVolume;
  bool signalValid;
} EngineModelState;

/***************** engineModelInit ********************************************
 * params: none
 * return: void
 * Description:
 * Initializes the engine model state.
 ******************************************************************************/
void engineModelInit(void);

/***************** engineModelUpdate ******************************************
 * params: uint16_t pulseWidthUs, bool signalValid, uint32_t nowMs
 * return: void
 * Description:
 * Updates throttle, RPM, and volume parameters based on RC input.
 ******************************************************************************/
void engineModelUpdate(uint16_t pulseWidthUs, bool signalValid, uint32_t nowMs);

/***************** engineModelGetState ****************************************
 * params: none
 * return: EngineModelState
 * Description:
 * Returns the current engine model state.
 ******************************************************************************/
EngineModelState engineModelGetState(void);

/***************** engineModelConsumeWastegateTrigger *************************
 * params: none
 * return: bool
 * Description:
 * Returns and clears the wastegate trigger flag.
 ******************************************************************************/
bool engineModelConsumeWastegateTrigger(void);

#endif
