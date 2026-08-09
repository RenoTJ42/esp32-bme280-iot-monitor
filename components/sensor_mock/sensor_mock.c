/**
 * @file    sensor_mock.c
 * @brief   Synthetic BME280 data generator — no hardware required
 *
 * Produces plausible, slowly-varying environmental readings so the
 * full task/queue/MQTT pipeline can be developed and tested before
 * the real BME280 hardware arrives.
 *
 * Implements the EXACT same bme280_read() signature the real I2C
 * driver will use — sensor_task is unaware this is mocked.
 */

#include "sensor_iface.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/* Slowly drifting state — makes the mock data look realistic in logs,
 * rather than returning a flat constant every call. */
static int32_t mock_temp  = 2300;      /* start at 23.00 °C  */
static uint32_t mock_hum   =  50 * 1024 / 100;  /* ~50.0 %RH  */
static uint32_t mock_press  =  101325;    /* standard pressure   */
static int8_t drift_dir  = 1;

bool bme280_read(bme280_reading_t *out)
{
    if (out == NULL){
        return false;
    }

    /* Drift temperature slowly between 22.00°C and 24.00°C
     * to simulate a real, slightly-changing room environment. */

    mock_temp += drift_dir;
    if (mock_temp >= 2400 || mock_temp <= 2200){
        drift_dir = -drift_dir;
    }

    out -> temperature_c_x100 = mock_temp;
    out->humidity_pct_x1024  = mock_hum;
    out->pressure_pa         = mock_press;
    out->timestamp_ms        = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    return true;    /* mock never fails */
}