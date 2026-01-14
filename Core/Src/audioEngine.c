#include "audioEngine.h"
#include "boardConfig.h"
#include "engineModel.h"
#include "soundData.h"
#include <stdbool.h>

#define AUDIO_DMA_FRAMES_PER_BUFFER (AUDIO_DMA_FRAMES_PER_HALF * 2U)
#define AUDIO_DMA_SAMPLES_PER_FRAME 2U
#define AUDIO_DMA_BUFFER_SAMPLES (AUDIO_DMA_FRAMES_PER_BUFFER * AUDIO_DMA_SAMPLES_PER_FRAME)

static I2S_HandleTypeDef *audioI2sHandle = NULL;
static int16_t audioDmaBuffer[AUDIO_DMA_BUFFER_SAMPLES];

static uint32_t enginePhase;
static uint32_t turboPhase;
static uint32_t fanPhase;
static uint32_t chargerPhase;
static uint32_t knockPhase;
static uint32_t wastegatePhase;
static uint32_t enginePhaseInc;
static uint32_t fixedPhaseInc;
static uint32_t wastegatePhaseInc;

static uint32_t lastEngineSampleIndex;
static uint32_t lastKnockSampleIndex;
static bool knockActive;
static bool wastegateActive;

static uint16_t idleVolume;
static uint16_t revVolume;
static uint16_t knockVolume;
static uint16_t turboVolume;
static uint16_t fanVolume;
static uint16_t chargerVolume;
static uint16_t wastegateVolume;
static uint16_t currentRpm;

static int32_t audioEngineClamp(int32_t value, int32_t minValue, int32_t maxValue)
{
  if (value < minValue)
  {
    return minValue;
  }
  if (value > maxValue)
  {
    return maxValue;
  }
  return value;
}

static int32_t audioEngineInterpolateLinear(const int8_t *samples, uint32_t count, uint32_t phase)
{
  if (samples == NULL || count == 0U)
  {
    return 0;
  }

  uint32_t index = phase >> 16U;
  uint32_t nextIndex = index + 1U;
  if (nextIndex >= count)
  {
    nextIndex = 0U;
  }

  int32_t sample0 = samples[index];
  int32_t sample1 = samples[nextIndex];
  uint32_t frac = phase & 0xFFFFU;
  int32_t delta = sample1 - sample0;
  return sample0 + (int32_t)((delta * (int32_t)frac) >> 16U);
}

static uint32_t audioEnginePhaseIncrement(uint32_t sourceRate)
{
  if (sourceRate == 0U)
  {
    return 0U;
  }

  uint64_t increment = ((uint64_t)sourceRate << 16U) / AUDIO_OUTPUT_SAMPLE_RATE;
  if (increment > 0xFFFFFFFFU)
  {
    increment = 0xFFFFFFFFU;
  }
  return (uint32_t)increment;
}

static void audioEngineAdvancePhase(uint32_t *phase, uint32_t increment, uint32_t count)
{
  if (count == 0U)
  {
    return;
  }

  uint32_t limit = count << 16U;
  uint32_t nextPhase = *phase + increment;
  while (nextPhase >= limit)
  {
    nextPhase -= limit;
  }
  *phase = nextPhase;
}

static int32_t audioEngineMixLayer(const int8_t *samples, uint32_t count, uint32_t *phase, uint32_t increment, uint16_t volumePercent)
{
  if (volumePercent == 0U)
  {
    audioEngineAdvancePhase(phase, increment, count);
    return 0;
  }

  int32_t sampleValue = audioEngineInterpolateLinear(samples, count, *phase);
  audioEngineAdvancePhase(phase, increment, count);
  return (sampleValue * (int32_t)volumePercent) / 100;
}

static void audioEngineTriggerKnock(uint32_t engineSampleIndex)
{
  if (!knockActive)
  {
    lastKnockSampleIndex = engineSampleIndex;
    knockActive = true;
    knockPhase = 0U;
  }
}

static int32_t audioEngineRenderFrame(void)
{
  uint32_t engineSampleIndex = enginePhase >> 16U;
  if (engineSampleIndex < lastEngineSampleIndex)
  {
    lastKnockSampleIndex = 0U;
    audioEngineTriggerKnock(engineSampleIndex);
  }

  if ((engineSampleIndex - lastKnockSampleIndex) > (sampleCount / (uint32_t)dieselKnockInterval))
  {
    audioEngineTriggerKnock(engineSampleIndex);
  }

  lastEngineSampleIndex = engineSampleIndex;

  int32_t idleSample = audioEngineInterpolateLinear(samples, sampleCount, enginePhase);
  int32_t idleLayer = (idleSample * (int32_t)idleVolume) / 100;
  int32_t a = idleLayer;

  if (revSampleCount > 0U)
  {
    int32_t revSample = audioEngineInterpolateLinear(revSamples, revSampleCount, enginePhase);
    int32_t revLayer = (revSample * (int32_t)revVolume) / 100;
    uint16_t idleBlend = (uint16_t)idleVolumeProportionPercentage;

    if (currentRpm > revSwitchPoint && idleEndPoint > revSwitchPoint)
    {
      uint16_t mapped = (uint16_t)(((uint32_t)idleVolumeProportionPercentage * (idleEndPoint - currentRpm)) /
        (idleEndPoint - revSwitchPoint));
      idleBlend = mapped;
    }

    if (currentRpm > idleEndPoint)
    {
      idleBlend = 0U;
    }

    idleLayer = (idleLayer * idleBlend) / 100;
    revLayer = (revLayer * (100U - idleBlend)) / 100;
    a = idleLayer + revLayer;
  }

  int32_t b = 0;
  if (knockActive)
  {
    uint32_t previousPhase = knockPhase;
    b += audioEngineMixLayer(knockSamples, knockSampleCount, &knockPhase, fixedPhaseInc, knockVolume);
    if (knockPhase < previousPhase)
    {
      knockActive = false;
    }
  }

  if (wastegateActive)
  {
    uint32_t previousPhase = wastegatePhase;
    b += audioEngineMixLayer(wastegateSamples, wastegateSampleCount, &wastegatePhase, wastegatePhaseInc, wastegateVolume);
    if (wastegatePhase < previousPhase)
    {
      wastegateActive = false;
    }
  }

  int32_t c = audioEngineMixLayer(turboSamples, turboSampleCount, &turboPhase, enginePhaseInc, turboVolume);
  int32_t d = audioEngineMixLayer(fanSamples, fanSampleCount, &fanPhase, enginePhaseInc, fanVolume);
  int32_t e = audioEngineMixLayer(chargerSamples, chargerSampleCount, &chargerPhase, enginePhaseInc, chargerVolume);
  int32_t f = 0;
  int32_t g = 0;

  audioEngineAdvancePhase(&enginePhase, enginePhaseInc, sampleCount);

  int32_t mix = (a * 8 / 10) + (b / 2) + (c / 5) + (d / 5) + (e / 5) + f + g;
  mix = (mix * (int32_t)masterVolume) / 100;

  int32_t sample16 = mix * 256;
  sample16 = audioEngineClamp(sample16, -32768, 32767);
  return sample16;
}

static void audioEngineRenderBuffer(int16_t *buffer, uint32_t frames)
{
  for (uint32_t i = 0U; i < frames; i++)
  {
    int32_t sample = audioEngineRenderFrame();
    buffer[i * 2U] = (int16_t)sample;
    buffer[i * 2U + 1U] = (int16_t)sample;
  }
}

/***************** audioEngineInit ********************************************
 * params: I2S_HandleTypeDef *i2sHandle
 * return: void
 * Description:
 * Initializes the audio engine and binds it to an I2S handle.
 ******************************************************************************/
void audioEngineInit(I2S_HandleTypeDef *i2sHandle)
{
  audioI2sHandle = i2sHandle;
  enginePhase = 0U;
  turboPhase = 0U;
  fanPhase = 0U;
  chargerPhase = 0U;
  knockPhase = 0U;
  wastegatePhase = 0U;
  enginePhaseInc = audioEnginePhaseIncrement(sampleRate);
  fixedPhaseInc = audioEnginePhaseIncrement(knockSampleRate);
  wastegatePhaseInc = audioEnginePhaseIncrement(wastegateSampleRate);
  lastEngineSampleIndex = 0U;
  lastKnockSampleIndex = 0U;
  knockActive = false;
  wastegateActive = false;

  idleVolume = (uint16_t)engineIdleVolumePercentage;
  revVolume = (uint16_t)engineRevVolumePercentage;
  knockVolume = (uint16_t)dieselKnockIdleVolumePercentage;
  turboVolume = (uint16_t)turboIdleVolumePercentage;
  fanVolume = (uint16_t)fanIdleVolumePercentage;
  chargerVolume = (uint16_t)chargerIdleVolumePercentage;
  wastegateVolume = (uint16_t)wastegateIdleVolumePercentage;
  currentRpm = 0U;
}

/***************** audioEngineStart *******************************************
 * params: none
 * return: void
 * Description:
 * Starts the I2S DMA playback using the internal double buffer.
 ******************************************************************************/
void audioEngineStart(void)
{
  if (audioI2sHandle == NULL)
  {
    return;
  }

  audioEngineRenderBuffer(audioDmaBuffer, AUDIO_DMA_FRAMES_PER_BUFFER);
  HAL_I2S_Transmit_DMA(audioI2sHandle, (uint16_t *)audioDmaBuffer, AUDIO_DMA_BUFFER_SAMPLES);
}

/***************** audioEngineControlTick *************************************
 * params: uint32_t nowMs
 * return: void
 * Description:
 * Updates DDS phase increments and gains based on the latest engine model state.
 ******************************************************************************/
void audioEngineControlTick(uint32_t nowMs)
{
  (void)nowMs;
  EngineModelState state = engineModelGetState();
  currentRpm = state.currentRpm;

  uint32_t engineSampleRate = sampleRate;
  if (state.engineSampleIntervalTicks != 0U)
  {
    engineSampleRate = 4000000U / state.engineSampleIntervalTicks;
  }
  enginePhaseInc = audioEnginePhaseIncrement(engineSampleRate);

  idleVolume = (uint16_t)((state.throttleDependentVolume * (uint32_t)idleVolumePercentage) / 100U);
  revVolume = (uint16_t)((state.throttleDependentRevVolume * (uint32_t)revVolumePercentage) / 100U);
  knockVolume = (uint16_t)((state.throttleDependentKnockVolume * (uint32_t)dieselKnockVolumePercentage) / 100U);
  turboVolume = (uint16_t)((state.throttleDependentTurboVolume * (uint32_t)turboVolumePercentage) / 100U);
  fanVolume = (uint16_t)((state.throttleDependentFanVolume * (uint32_t)fanVolumePercentage) / 100U);
  chargerVolume = (uint16_t)((state.throttleDependentChargerVolume * (uint32_t)chargerVolumePercentage) / 100U);
  wastegateVolume = (uint16_t)((state.rpmDependentWastegateVolume * (uint32_t)wastegateVolumePercentage) / 100U);

  if (engineModelConsumeWastegateTrigger())
  {
    wastegateActive = true;
    wastegatePhase = 0U;
  }
}

/***************** audioEngineHandleHalfTransfer ******************************
 * params: I2S_HandleTypeDef *i2sHandle
 * return: void
 * Description:
 * Handles the DMA half-transfer callback to render audio samples.
 ******************************************************************************/
void audioEngineHandleHalfTransfer(I2S_HandleTypeDef *i2sHandle)
{
  if (i2sHandle != audioI2sHandle)
  {
    return;
  }

  audioEngineRenderBuffer(audioDmaBuffer, AUDIO_DMA_FRAMES_PER_HALF);
}

/***************** audioEngineHandleFullTransfer ******************************
 * params: I2S_HandleTypeDef *i2sHandle
 * return: void
 * Description:
 * Handles the DMA full-transfer callback to render audio samples.
 ******************************************************************************/
void audioEngineHandleFullTransfer(I2S_HandleTypeDef *i2sHandle)
{
  if (i2sHandle != audioI2sHandle)
  {
    return;
  }

  audioEngineRenderBuffer(&audioDmaBuffer[AUDIO_DMA_FRAMES_PER_HALF * AUDIO_DMA_SAMPLES_PER_FRAME],
    AUDIO_DMA_FRAMES_PER_HALF);
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
  audioEngineHandleHalfTransfer(hi2s);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
  audioEngineHandleFullTransfer(hi2s);
}
