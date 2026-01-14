#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Board selection
#define BOARD_NUCLEO_F303RE 1

// PWM input timer configuration (TIM2 CH2+CH3, TIM3 CH1+CH2)
#define RC_INPUT_TIMER_INSTANCE TIM2
#define RC_INPUT_TIMER_CHANNEL_1 TIM_CHANNEL_2
#define RC_INPUT_TIMER_CHANNEL_2 TIM_CHANNEL_3
#define RC_INPUT_TIMER_CHANNEL_3 TIM_CHANNEL_1
#define RC_INPUT_TIMER_CHANNEL_4 TIM_CHANNEL_2
#define RC_INPUT_TIMER_HZ 1000000U

// PWM input pins (Nucleo-64 STM32G474RE)
#define RC_INPUT_CH1_PIN GPIO_PIN_3
#define RC_INPUT_CH1_PORT GPIOB
#define RC_INPUT_CH2_PIN GPIO_PIN_4
#define RC_INPUT_CH2_PORT GPIOB
#define RC_INPUT_CH3_PIN GPIO_PIN_5
#define RC_INPUT_CH3_PORT GPIOB
#define RC_INPUT_CH4_PIN GPIO_PIN_10
#define RC_INPUT_CH4_PORT GPIOB

// I2S configuration
#define AUDIO_I2S_INSTANCE SPI2

// DMA settings
#define AUDIO_DMA_FRAMES_PER_HALF 256U

// Audio sample rate
// #define AUDIO_SAMPLE_RATE_44100

#ifdef AUDIO_SAMPLE_RATE_44100
#define AUDIO_OUTPUT_SAMPLE_RATE 44100U
#else
#define AUDIO_OUTPUT_SAMPLE_RATE 22050U
#endif

// PWM input limits
#define RC_PULSE_MIN_US 1000U
#define RC_PULSE_MAX_US 2000U
#define RC_PULSE_NEUTRAL_US 1500U
#define RC_PULSE_DEADBAND_US 20U

// Failsafe
#define RC_FAILSAFE_TIMEOUT_MS 50U

// Notes for STM32G431/G474 migration:
// - TIM2 is 32-bit on G4 as well, but pin mapping differs (use PA0..PA3 or alternate pins).
// - SPI2/I2S2 pin mapping changes; reassign in CubeMX and update this file.
// - DMA channel mappings differ; reconfigure DMA requests.

#endif
