#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "stm32g4xx_hal.h"
#include <stdint.h>

/***************** audioEngineInit ********************************************
 * params: I2S_HandleTypeDef *i2sHandle
 * return: void
 * Description:
 * Initializes the audio engine and binds it to an I2S handle.
 ******************************************************************************/
void audioEngineInit(I2S_HandleTypeDef *i2sHandle);

/***************** audioEngineStart *******************************************
 * params: none
 * return: void
 * Description:
 * Starts the I2S DMA playback using the internal double buffer.
 ******************************************************************************/
void audioEngineStart(void);

/***************** audioEngineControlTick *************************************
 * params: uint32_t nowMs
 * return: void
 * Description:
 * Updates DDS phase increments and gains based on the latest engine model state.
 ******************************************************************************/
void audioEngineControlTick(uint32_t nowMs);

/***************** audioEngineHandleHalfTransfer ******************************
 * params: I2S_HandleTypeDef *i2sHandle
 * return: void
 * Description:
 * Handles the DMA half-transfer callback to render audio samples.
 ******************************************************************************/
void audioEngineHandleHalfTransfer(I2S_HandleTypeDef *i2sHandle);

/***************** audioEngineHandleFullTransfer ******************************
 * params: I2S_HandleTypeDef *i2sHandle
 * return: void
 * Description:
 * Handles the DMA full-transfer callback to render audio samples.
 ******************************************************************************/
void audioEngineHandleFullTransfer(I2S_HandleTypeDef *i2sHandle);

#endif
