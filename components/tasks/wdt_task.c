/**
 * @file    wdt_task.c
 * @brief   Watchdog and diagnostics task — Core 0, Priority 3
 *
 * Runs every 1000ms. Checks:
 *   - Stack high-water marks for sensor and mqtt tasks
 *   - Heap free space
 *   - Logs diagnostics over UART
 *
 * In Phase 10 this task also feeds the hardware watchdog timer.
 * For now it monitors and reports only.
 */

#include "wdt_task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "WDT";

void wdt_task(void *pvParameters)
{
    wdt_params_t *params = (wdt_params_t *)pvParameters;

    ESP_LOGI(TAG, "Watchdog task started on Core %d", xPortGetCoreID());

    while (1) {
        UBaseType_t sensor_watermark = uxTaskGetStackHighWaterMark(params->sensor_handle);
        UBaseType_t mqtt_watermark   = uxTaskGetStackHighWaterMark(params->mqtt_handle);
        uint32_t    heap_free        = (uint32_t)heap_caps_get_free_size(MALLOC_CAP_8BIT);

        ESP_LOGI(TAG, "--- Diagnostics ---");
        ESP_LOGI(TAG, "  sensor_task stack free : %u words", sensor_watermark);
        ESP_LOGI(TAG,   "  mqtt_task   stack free : %u words", mqtt_watermark);
        ESP_LOGI(TAG, "  Heap free              : %lu bytes", (unsigned long)heap_free);

        /* Warn if any stack is critically low (< 128 words = 512 bytes) */
        if (sensor_watermark < 128 || mqtt_watermark < 128) {
            ESP_LOGW(TAG, "WARNING: Stack space critically low!");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
