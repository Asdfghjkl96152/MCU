#include "inference.h"
#include <math.h>
#include <stdio.h>

/* Simple RMS energy-based detector as placeholder.
   Replace with TFLM integration later.
*/

bool inference_init(void)
{
    // allocate resources if needed
    return true;
}

bool inference_run_on_audio(const int16_t *pcm_samples, size_t nsamples)
{
    if (!pcm_samples || nsamples == 0) return false;
    uint64_t acc = 0;
    for (size_t i = 0; i < nsamples; ++i) {
        int32_t s = pcm_samples[i];
        acc += (uint64_t)(s * s);
    }
    double mean = (double)acc / (double)nsamples;
    double rms = sqrt(mean);
    const double THRESH = 1000.0; // tune as needed
    if (rms > THRESH) {
        return true;
    }
    return false;
}

void inference_deinit(void)
{
}
