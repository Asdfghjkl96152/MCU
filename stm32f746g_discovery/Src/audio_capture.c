#include "audio_capture.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "stm32f7xx_hal.h"

extern I2S_HandleTypeDef hi2s2; // ensure your CubeMX generates this (or change name)
static uint8_t *s_capture_buffer = NULL;
static size_t s_capture_buffer_bytes = 0;
static SemaphoreHandle_t s_frame_sem = NULL;
static size_t s_frame_offset = 0; // offset where current complete frame ends
static uint32_t s_half_done = 0;
static uint32_t s_full_done = 0;

#define DMA_TOTAL_BUF_SAMPLES (AUDIO_CAPTURE_FRAME_SAMPLES * 2) // double to reduce interrupts
#define DMA_TOTAL_BUF_BYTES (DMA_TOTAL_BUF_SAMPLES * (AUDIO_CAPTURE_BITS_PER_SAMPLE/8))

/* Provide weak callback handlers that CubeMX may generate; we hook IRQ callbacks */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    (void)hi2s;
    s_half_done = 1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (s_frame_sem) xSemaphoreGiveFromISR(s_frame_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    (void)hi2s;
    s_full_done = 1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (s_frame_sem) xSemaphoreGiveFromISR(s_frame_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Initialize capture: allocate DMA buffer and start DMA receive in circular mode */
HAL_StatusTypeDef audio_capture_init(const audio_capture_config_t *cfg)
{
    if (!cfg) return HAL_ERROR;
    // Ensure sample rate matches peripheral config (configured in CubeMX)
    s_capture_buffer_bytes = DMA_TOTAL_BUF_BYTES;
    s_capture_buffer = pvPortMalloc(s_capture_buffer_bytes);
    if (!s_capture_buffer) return HAL_ERROR;

    s_frame_sem = xSemaphoreCreateBinary();
    if (!s_frame_sem) {
        vPortFree(s_capture_buffer);
        return HAL_ERROR;
    }

    // Start I2S DMA in circular mode
    if (HAL_I2S_Receive_DMA(&hi2s2, (uint16_t *)s_capture_buffer, DMA_TOTAL_BUF_SAMPLES) != HAL_OK) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/* Blocking read: copy a frame worth of bytes into out_buffer. We wait for half/full callbacks. */
HAL_StatusTypeDef audio_capture_get_frame(uint8_t *out_buffer, size_t frame_bytes, uint32_t timeout_ms)
{
    if (!out_buffer) return HAL_ERROR;
    uint32_t start = HAL_GetTick();
    size_t copied = 0;
    while (copied < frame_bytes) {
        // wait for an interrupt indicating half or full buffer ready
        if (xSemaphoreTake(s_frame_sem, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
            // check which half completed
            if (s_half_done) {
                // copy first half
                memcpy(out_buffer + copied, s_capture_buffer, frame_bytes - copied > (s_capture_buffer_bytes/2) ? (s_capture_buffer_bytes/2) : (frame_bytes - copied));
                s_half_done = 0;
                copied += s_capture_buffer_bytes/2;
            } else if (s_full_done) {
                // copy second half
                memcpy(out_buffer + copied, s_capture_buffer + (s_capture_buffer_bytes/2),
                       frame_bytes - copied > (s_capture_buffer_bytes/2) ? (s_capture_buffer_bytes/2) : (frame_bytes - copied));
                s_full_done = 0;
                copied += s_capture_buffer_bytes/2;
            }
        } else {
            // timeout
            return HAL_TIMEOUT;
        }

        if ((HAL_GetTick() - start) > timeout_ms) return HAL_TIMEOUT;
    }
    return HAL_OK;
}

void audio_capture_deinit(void)
{
    if (s_capture_buffer) {
        vPortFree(s_capture_buffer);
        s_capture_buffer = NULL;
    }
    if (s_frame_sem) {
        vSemaphoreDelete(s_frame_sem);
        s_frame_sem = NULL;
    }
    HAL_I2S_DMAStop(&hi2s2);
}
