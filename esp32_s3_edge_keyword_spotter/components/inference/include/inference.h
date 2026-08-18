#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "audio_capture.h"

#ifdef __cplusplus
extern "C" {
#endif

bool inference_init(void);
bool inference_run_on_audio(const int16_t *pcm_samples, size_t nsamples);
void inference_deinit(void);

#ifdef __cplusplus
}
#endif
