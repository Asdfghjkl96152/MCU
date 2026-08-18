#include "audio_capture.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "audio_capture";

/* I2S config - may need to adjust pins for your board */
static i2s_port_t i2s_num = I2S_NUM_0;

static bool s_initialized = false;

esp_err_t audio_capture_init(const audio_capture_config_t *cfg)
{
    if (!cfg) return ESP_ERR_INVALID_ARG;

    // I2S configuration for Master Rx, 16-bit, 16kHz
    i2s_config_t i2s_cfg = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = cfg->sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // mono
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_cfg = {
        .bck_io_num = 13,   // SCK
        .ws_io_num = 12,    // WS
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 15,  // SD
    };

    esp_err_t err = i2s_driver_install(i2s_num, &i2s_cfg, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2s_driver_install failed: %d", err);
        return err;
    }
    err = i2s_set_pin(i2s_num, &pin_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2s_set_pin failed: %d", err);
        return err;
    }
    // optional: set clock if needed
    s_initialized = true;
    ESP_LOGI(TAG, "I2S init done");
    return ESP_OK;
}

esp_err_t audio_capture_get_frame(uint8_t *out_buffer, size_t frame_bytes, TickType_t timeout_ticks)
{
    if (!s_initialized || !out_buffer) return ESP_ERR_INVALID_STATE;

    size_t bytes_read = 0;
    esp_err_t err = i2s_read(i2s_num, out_buffer, frame_bytes, &bytes_read, timeout_ticks);
    if (err != ESP_OK) {
        return err;
    }
    if (bytes_read < frame_bytes) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

void audio_capture_deinit(void)
{
    if (s_initialized) {
        i2s_driver_uninstall(i2s_num);
        s_initialized = false;
    }
}
