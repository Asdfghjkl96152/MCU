/*
 inference.c
 - Default: energy-based stub (computes RMS)
 - Placeholder for TFLM integration: include model_data.c, TFLM headers and implement preprocessing + invoke
*/

#include "inference.h"
#include <stdlib.h>
#include <math.h>
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "arm_math.h" // optional if using CMSIS DSP

bool inference_init(void)
{
    // Initialize TFLM / CMSIS resources here when integrated
    return true;
}

bool inference_run_on_audio(const int16_t *pcm_samples, size_t nsamples)
{
    // energy-based simple detector (stub)
    uint64_t acc = 0;
    for (size_t i = 0; i < nsamples; ++i) {
        int32_t s = pcm_samples[i];
        acc += (uint64_t)(s * s);
    }
    double mean = (double)acc / (double)nsamples;
    double rms = sqrt(mean);
    const double THRESH = 1000.0;
    if (rms > THRESH) {
        return true;
    }
    return false;
}

void inference_deinit(void)
{
}
