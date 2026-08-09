/**
 * @file    mqtt_task.c
 * @brief   MQTT publish task (stub) — Core 1, Priority 5
 *
 * Phase 5: Receives readings from queue and logs as JSON over UART.
 * Phase 9: esp_mqtt_client_publish() replaces the ESP_LOGI stub.
 *
 * Pinned to Core 1 to isolate TLS/MQTT CPU load from sensor task.
 */

#include "mqtt_task.h"
#include "sensor_iface.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "MQTT";

void mqtt_task(void *pvParameters)
{
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    bme280_reading_t reading;

    ESP_LOGI(TAG, "MQTT task started on Core %d", xPortGetCoreID());

    while (1) {
        /* Block indefinitely until sensor_task overwrites queue.
         * portMAX_DELAY = wait forever — no timeout needed since
         * sensor_task runs continuously and will always produce data. */
        if (xQueueReceive(queue, &reading, portMAX_DELAY) == pdTRUE) {

            /* Phase 5 stub: format as JSON and log.
             * Phase 9: replace this block with:
             *   esp_mqtt_client_publish(client,
             *       "esp32/sensors/bme280", json_buf, len, 1, 0); */
            ESP_LOGI(TAG,
                "{\"temp\":%ld.%02ld,\"hum\":%lu,\"press\":%lu,\"ts\":%lu}",
                (long)(reading.temperature_c_x100 / 100),
                (long)(reading.temperature_c_x100 % 100),
                (unsigned long)(reading.humidity_pct_x1024 * 100 / 1024),
                (unsigned long)reading.pressure_pa,
                (unsigned long)reading.timestamp_ms);
        }
    }
}
