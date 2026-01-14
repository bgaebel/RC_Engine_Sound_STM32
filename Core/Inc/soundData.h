#ifndef SOUND_DATA_H
#define SOUND_DATA_H

#include <stdint.h>

extern const int8_t samples[];
extern const unsigned int sampleCount;
extern const unsigned int sampleRate;

extern const int8_t revSamples[];
extern const unsigned int revSampleCount;
extern const unsigned int revSampleRate;

extern const int8_t knockSamples[];
extern const unsigned int knockSampleCount;
extern const unsigned int knockSampleRate;

extern const int8_t turboSamples[];
extern const unsigned int turboSampleCount;
extern const unsigned int turboSampleRate;

extern const int8_t fanSamples[];
extern const unsigned int fanSampleCount;
extern const unsigned int fanSampleRate;

extern const int8_t chargerSamples[];
extern const unsigned int chargerSampleCount;
extern const unsigned int chargerSampleRate;

extern const int8_t wastegateSamples[];
extern const unsigned int wastegateSampleCount;
extern const unsigned int wastegateSampleRate;

extern uint32_t idleVolumePercentage;
extern uint32_t engineIdleVolumePercentage;
extern uint32_t fullThrottleVolumePercentage;
extern uint32_t revVolumePercentage;
extern uint32_t engineRevVolumePercentage;
extern volatile const uint32_t revSwitchPoint;
extern volatile const uint32_t idleEndPoint;
extern volatile const uint32_t idleVolumeProportionPercentage;

extern volatile int dieselKnockVolumePercentage;
extern volatile int dieselKnockIdleVolumePercentage;
extern volatile int dieselKnockStartPoint;
extern volatile int dieselKnockInterval;
extern volatile int dieselKnockAdaptiveVolumePercentage;

extern volatile int turboVolumePercentage;
extern volatile int turboIdleVolumePercentage;
extern volatile int fanVolumePercentage;
extern volatile int fanIdleVolumePercentage;
extern volatile int fanStartPoint;
extern volatile int chargerVolumePercentage;
extern volatile int chargerIdleVolumePercentage;
extern volatile int chargerStartPoint;
extern volatile int wastegateVolumePercentage;
extern volatile int wastegateIdleVolumePercentage;

extern uint32_t MAX_RPM_PERCENTAGE;
extern const int8_t acc;
extern const int8_t dec;
extern uint16_t masterVolume;

#endif
