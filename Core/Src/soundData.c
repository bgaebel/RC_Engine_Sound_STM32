#include "soundData.h"

// Core engine sound assets (Volvo L120H configuration)
#include "../../src/vehicles/sounds/L120H_BaumaIdleOutside.h"
#include "../../src/vehicles/sounds/L120H_BaumaKnock.h"
#include "../../src/vehicles/sounds/TurboWhistle.h"
#include "../../src/vehicles/sounds/GenericFan.h"
#include "../../src/vehicles/sounds/supercharger.h"
#include "../../src/vehicles/sounds/UnimogU1000TurboWastegate.h"

const SoundData soundData = {
  .engineSamples = samples,
  .engineSampleCount = sampleCount,
  .engineSampleRate = sampleRate,
  .revSamples = NULL,
  .revSampleCount = 0U,
  .revSampleRate = 0U,
  .knockSamples = knockSamples,
  .knockSampleCount = knockSampleCount,
  .knockSampleRate = knockSampleRate,
  .turboSamples = turboSamples,
  .turboSampleCount = turboSampleCount,
  .turboSampleRate = turboSampleRate,
  .fanSamples = fanSamples,
  .fanSampleCount = fanSampleCount,
  .fanSampleRate = fanSampleRate,
  .chargerSamples = chargerSamples,
  .chargerSampleCount = chargerSampleCount,
  .chargerSampleRate = chargerSampleRate,
  .wastegateSamples = wastegateSamples,
  .wastegateSampleCount = wastegateSampleCount,
  .wastegateSampleRate = wastegateSampleRate,
  .idleVolumePercentage = 80U,
  .engineIdleVolumePercentage = 80U,
  .fullThrottleVolumePercentage = 140U,
  .revVolumePercentage = 110U,
  .engineRevVolumePercentage = 50U,
  .revSwitchPoint = 100U,
  .idleEndPoint = 400U,
  .idleVolumeProportionPercentage = 90U,
  .dieselKnockVolumePercentage = 80U,
  .dieselKnockIdleVolumePercentage = 10U,
  .dieselKnockStartPoint = 110U,
  .dieselKnockInterval = 6U,
  .dieselKnockAdaptiveVolumePercentage = 50U,
  .turboVolumePercentage = 0U,
  .turboIdleVolumePercentage = 0U,
  .fanVolumePercentage = 0U,
  .fanIdleVolumePercentage = 0U,
  .fanStartPoint = 0U,
  .chargerVolumePercentage = 0U,
  .chargerIdleVolumePercentage = 10U,
  .chargerStartPoint = 10U,
  .wastegateVolumePercentage = 50U,
  .wastegateIdleVolumePercentage = 1U,
  .maxRpmPercentage = 200U,
  .accelerationStep = 6U,
  .decelerationStep = 3U,
  .masterVolume = 100U
};
