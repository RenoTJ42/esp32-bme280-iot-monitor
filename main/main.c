/**
 * @file    main.c
 * @brief   Secure IoT ESP32 — Application Entry Point
 *
 * Orchestrates the three-task FreeRTOS architecture:
 *   - sensor_task  (Core 0, Priority 4): BME280 mock poll -> queue
 *   - mqtt_task    (Core 1, Priority 5): queue -> JSON log (stub)
 *   - wdt_task     (Core 0, Priority 3): stack/heap diagnostics
 *
 * Phase 5: mock sensor data, MQTT stub (log only).
 * Phase 9: real BME280 + TLS + MQTT publish replaces stubs.
 */

#include <stdio.h>
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_idf_version.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sensor_iface.h"
#include "uart_driver.h"
#include "gpio_driver.h"
#include "sensor_task.h"
#include "mqtt_task.h"
#include "wdt_task.h"

static const char *TAG = "MAIN";

/* ── Task stack sizes ────────────────────────────────────────────── */
#define SENSOR_STACK_SIZE   4096
#define MQTT_STACK_SIZE     8192
#define WDT_STACK_SIZE      2048

/* ── Task priorities ─────────────────────────────────────────────── */
#define SENSOR_PRIORITY     4
#define MQTT_PRIORITY       5
#define WDT_PRIORITY        3

/* ── Core assignments ────────────────────────────────────────────── */
#define SENSOR_CORE         0
#define MQTT_CORE           1
#define WDT_CORE            0

/* ── Shared queue — length 1, xQueueOverwrite() pattern ─────────── */
static QueueHandle_t reading_queue = NULL;

/* ── Task handles — needed by wdt_task for stack monitoring ─────── */
static TaskHandle_t  sensor_handle = NULL;
static TaskHandle_t  mqtt_handle   = NULL;

/* ── wdt_task parameters — static lifetime (never goes out of scope) */
static wdt_params_t  wdt_params;

/**
 * @brief Verify our custom certs partition is accessible at runtime.
 */
static void verify_partitions(void)
{
    ESP_LOGI(TAG, "--- Partition Table ---");
    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it != NULL) {
        const esp_partition_t *p = esp_partition_get(it);
        ESP_LOGI(TAG, "  [%-12s] type=0x%02x sub=0x%02x "
                      "offset=0x%08"PRIx32" size=0x%08"PRIx32,
                 p->label, p->type, p->subtype, p->address, p->size);
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);

    const esp_partition_t *certs = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, 0xFF, "certs");
    if (certs == NULL) {
        ESP_LOGE(TAG, "FATAL: certs partition not found");
    } else {
        ESP_LOGI(TAG, "OK: certs partition @ 0x%08"PRIx32, certs->address);
    }
}

void app_main(void)
{
    /* ── Drivers ─────────────────────────────────────────────────── */
    uart_driver_init();
    uart_write_str("\r\n=== Secure IoT ESP32 — Phase 5 Boot ===\r\n");

    gpio_driver_init();
    gpio_led_blink(100);   /* brief boot flash */

    /* ── System info ─────────────────────────────────────────────── */
    ESP_LOGI(TAG, "ESP-IDF %s | 2-core LX6 @ 160MHz", esp_get_idf_version());
    verify_partitions();

    /* ── Queue ───────────────────────────────────────────────────── *
     * Length=1 + xQueueOverwrite() = mailbox pattern.               *
     * sensor_task always writes freshest reading; never blocks.     *
     * mqtt_task always reads freshest reading when it wakes.        */
    reading_queue = xQueueCreate(1, sizeof(bme280_reading_t));
    if (reading_queue == NULL) {
        ESP_LOGE(TAG, "FATAL: queue creation failed — out of heap");
        return;
    }
    ESP_LOGI(TAG, "Queue created (length=1, item=%u bytes)",
             (unsigned)sizeof(bme280_reading_t));

    /* ── Tasks ───────────────────────────────────────────────────── */
    BaseType_t rc;

    rc = xTaskCreatePinnedToCore(
        sensor_task,       /* function                */
        "sensor",          /* debug name              */
        SENSOR_STACK_SIZE, /* stack bytes             */
        (void *)reading_queue, /* parameter: queue handle */
        SENSOR_PRIORITY,   /* priority                */
        &sensor_handle,    /* handle out              */
        SENSOR_CORE        /* core                    */
    );
    if (rc != pdPASS) {
        ESP_LOGE(TAG, "FATAL: sensor_task creation failed");
        return;
    }

    rc = xTaskCreatePinnedToCore(
        mqtt_task,
        "mqtt",
        MQTT_STACK_SIZE,
        (void *)reading_queue,
        MQTT_PRIORITY,
        &mqtt_handle,
        MQTT_CORE
    );
    if (rc != pdPASS) {
        ESP_LOGE(TAG, "FATAL: mqtt_task creation failed");
        return;
    }

    /* wdt_task needs both handles — pack into static struct */
    wdt_params.sensor_handle = sensor_handle;
    wdt_params.mqtt_handle   = mqtt_handle;

    rc = xTaskCreatePinnedToCore(
        wdt_task,
        "wdt",
        WDT_STACK_SIZE,
        (void *)&wdt_params,
        WDT_PRIORITY,
        NULL,              /* we don't need wdt's own handle */
        WDT_CORE
    );
    if (rc != pdPASS) {
        ESP_LOGE(TAG, "FATAL: wdt_task creation failed");
        return;
    }

    ESP_LOGI(TAG, "All tasks spawned. Scheduler running.");
    ESP_LOGI(TAG, "  sensor_task : Core %d  Priority %d  Stack %d",
             SENSOR_CORE, SENSOR_PRIORITY, SENSOR_STACK_SIZE);
    ESP_LOGI(TAG, "  mqtt_task   : Core %d  Priority %d  Stack %d",
             MQTT_CORE, MQTT_PRIORITY, MQTT_STACK_SIZE);
    ESP_LOGI(TAG, "  wdt_task    : Core %d  Priority %d  Stack %d",
             WDT_CORE, WDT_PRIORITY, WDT_STACK_SIZE);

    /* app_main() returns here — FreeRTOS scheduler takes over.
     * All three tasks continue running independently. */
}
