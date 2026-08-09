/**
 * @file    sensor_task.c
 * @brief   Sensor acquisition task — Core 0, Priority 4
 *
 * Polls bme280_read() every 500ms and overwrites the shared queue
 * with the latest reading. Uses xQueueOverwrite() so it never blocks
 * even if mqtt_task is slow (TLS handshake, reconnect, etc.).
 */

#include "sensor_task.h"
#include "sensor_iface.h"
#include "gpio_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "SENSOR";

void sensor_task(void *pvParameters)
{
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    bme280_reading_t reading;

    ESP_LOGI(TAG, "Sensor task started on Core %d", xPortGetCoreID());

    while (1) {
        if (bme280_read(&reading)) {
            /* xQueueOverwrite: always succeeds, never blocks.
             * Drops old unread value if queue is full (length=1).
             * Ensures mqtt_task always gets FRESHEST reading. */
            xQueueOverwrite(queue, &reading);

            ESP_LOGI(TAG, "T=%ld.%02ld°C  H=%lu%%  P=%luPa  ts=%lums",
                     (long)(reading.temperature_c_x100 / 100),
                     (long)(reading.temperature_c_x100 % 100),
                     (unsigned long)(reading.humidity_pct_x1024 * 100 / 1024),
                     (unsigned long)reading.pressure_pa,
                     (unsigned long)reading.timestamp_ms);

            /* Heartbeat blink — visual system-alive indicator */
            gpio_led_on();
        } else {
            ESP_LOGE(TAG, "Sensor read failed");
            gpio_led_off();
        }

        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_led_off();   /* off after 500ms delay — creates blink pattern */
    }
}
