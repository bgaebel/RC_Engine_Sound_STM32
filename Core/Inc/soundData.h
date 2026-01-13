#ifndef SOUND_DATA_H
#define SOUND_DATA_H

#include <stdint.h>

typedef struct
{
  const int8_t *engineSamples;
  uint32_t engineSampleCount;
  uint32_t engineSampleRate;
  const int8_t *revSamples;
  uint32_t revSampleCount;
  uint32_t revSampleRate;
  const int8_t *knockSamples;
  uint32_t knockSampleCount;
  uint32_t knockSampleRate;
  const int8_t *turboSamples;
  uint32_t turboSampleCount;
  uint32_t turboSampleRate;
  const int8_t *fanSamples;
  uint32_t fanSampleCount;
  uint32_t fanSampleRate;
  const int8_t *chargerSamples;
  uint32_t chargerSampleCount;
  uint32_t chargerSampleRate;
  const int8_t *wastegateSamples;
  uint32_t wastegateSampleCount;
  uint32_t wastegateSampleRate;
  uint16_t idleVolumePercentage;
  uint16_t engineIdleVolumePercentage;
  uint16_t fullThrottleVolumePercentage;
  uint16_t revVolumePercentage;
  uint16_t engineRevVolumePercentage;
  uint16_t revSwitchPoint;
  uint16_t idleEndPoint;
  uint16_t idleVolumeProportionPercentage;
  uint16_t dieselKnockVolumePercentage;
  uint16_t dieselKnockIdleVolumePercentage;
  uint16_t dieselKnockStartPoint;
  uint16_t dieselKnockInterval;
  uint16_t dieselKnockAdaptiveVolumePercentage;
  uint16_t turboVolumePercentage;
  uint16_t turboIdleVolumePercentage;
  uint16_t fanVolumePercentage;
  uint16_t fanIdleVolumePercentage;
  uint16_t fanStartPoint;
  uint16_t chargerVolumePercentage;
  uint16_t chargerIdleVolumePercentage;
  uint16_t chargerStartPoint;
  uint16_t wastegateVolumePercentage;
  uint16_t wastegateIdleVolumePercentage;
  uint32_t maxRpmPercentage;
  uint16_t accelerationStep;
  uint16_t decelerationStep;
  uint16_t masterVolume;
} SoundData;

extern const SoundData soundData;

#endif
