#pragma once
#include "stdint.h"
#include "stm32f7xx_hal.h"

#define AUDIO_CAPTURE_SAMPLE_RATE 16000
#define AUDIO_CAPTURE_BITS_PER_SAMPLE 16
#define AUDIO_CAPTURE_CHANNELS 1
#define AUDIO_CAPTURE_FRAME_MS 512
#define AUDIO_CAPTURE_FRAME_SAMPLES ((AUDIO_CAPTURE_SAMPLE_RATE * AUDIO_CAPTURE_FRAME_MS) / 1000)
#define AUDIO_CAPTURE_FRAME_BYTES (AUDIO_CAPTURE_FRAME_SAMPLES * (AUDIO_CAPTURE_BITS_PER_SAMPLE/8))

typedef struct {
    int sample_rate;
    int bits_per_sample;
    int channels;
} audio_capture_config_t;

/* Initialize audio capture (I2S + DMA). Must be called after HAL init and peripheral init. */
HAL_StatusTypeDef audio_capture_init(const audio_capture_config_t *cfg);

/* Blocking read: fills out_buffer with frame_bytes bytes. timeout_ms uses HAL_GetTick based waiting. */
HAL_StatusTypeDef audio_capture_get_frame(uint8_t *out_buffer, size_t frame_bytes, uint32_t timeout_ms);

/* Deinit */
void audio_capture_deinit(void);
