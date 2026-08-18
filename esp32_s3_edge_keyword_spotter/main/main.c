#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "audio_capture.h"
#include "inference.h"
#include "driver/gpio.h"
#include <stdlib.h>
#include <stdio.h>

#define AUDIO_QUEUE_LEN 4

static QueueHandle_t audio_queue = NULL;

/* Adjust this LED pin to your board (many ESP32 devkits use GPIO2 / 13 etc.) */
#define LED_GPIO GPIO_NUM_2

static void audio_producer_task(void *arg)
{
    (void)arg;
    size_t frame_bytes = AUDIO_CAPTURE_FRAME_BYTES;
    uint8_t *buf = malloc(frame_bytes);
    if (!buf) {
        printf("malloc fail\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        vTaskDelete(NULL);
    }

    while (1) {
        if (audio_capture_get_frame(buf, frame_bytes, portMAX_DELAY) == ESP_OK) {
            uint8_t *send_buf = malloc(frame_bytes);
            if (!send_buf) {
                // drop frame if cannot allocate
                continue;
            }
            memcpy(send_buf, buf, frame_bytes);
            if (xQueueSend(audio_queue, &send_buf, pdMS_TO_TICKS(100)) != pdTRUE) {
                // queue full; drop
                free(send_buf);
            }
        } else {
            // timeout or error
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

static void inference_consumer_task(void *arg)
{
    (void)arg;
    for (;;) {
        uint8_t *recv_buf = NULL;
        if (xQueueReceive(audio_queue, &recv_buf, portMAX_DELAY) == pdTRUE) {
            bool detected = inference_run_on_audio((const int16_t *)recv_buf, AUDIO_CAPTURE_FRAME_SAMPLES);
            if (detected) {
                printf("[DETECT] keyword detected\n");
                gpio_set_level(LED_GPIO, 1);
                vTaskDelay(pdMS_TO_TICKS(150));
                gpio_set_level(LED_GPIO, 0);
            }
            free(recv_buf);
        }
    }
}

void app_main(void)
{
    // init LED
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // init audio capture
    audio_capture_config_t cfg = {
        .sample_rate = AUDIO_CAPTURE_SAMPLE_RATE,
        .bits_per_sample = AUDIO_CAPTURE_BITS_PER_SAMPLE,
        .channels = AUDIO_CAPTURE_CHANNELS,
    };

    if (audio_capture_init(&cfg) != ESP_OK) {
        printf("audio_capture_init failed\n");
        return;
    }

    if (!inference_init()) {
        printf("inference_init failed\n");
        return;
    }

    audio_queue = xQueueCreate(AUDIO_QUEUE_LEN, sizeof(uint8_t *));
    if (!audio_queue) {
        printf("queue create failed\n");
        return;
    }

    xTaskCreate(audio_producer_task, "audio_prod", 4 * 1024, NULL, 6, NULL);
    xTaskCreate(inference_consumer_task, "inf_cons", 6 * 1024, NULL, 5, NULL);
}
