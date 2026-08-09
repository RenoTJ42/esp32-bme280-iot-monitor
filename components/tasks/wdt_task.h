#ifndef WDT_TASK_H
#define WDT_TASK_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
typedef struct {
    TaskHandle_t sensor_handle;
    TaskHandle_t mqtt_handle;
} wdt_params_t;
void wdt_task(void *pvParameters);
#endif
