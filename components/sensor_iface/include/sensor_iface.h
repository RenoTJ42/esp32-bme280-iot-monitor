/**
 * @file    sensor_iface.h
 * @brief   Hardware-agnostic sensor data contract
 *
 * This struct and function signature define the boundary between
 * sensor_task (consumer-agnostic) and the actual sensor driver
 * (currently mocked, later real I2C/BME280).
 *
 * DESIGN: sensor_task only ever calls bme280_read(). It never knows
 * whether the data came from I2C registers or a mock generator.
 */


#ifndef SENSOR_IFACE_H
#define SENSOR_IFACE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief One complete environmental reading. 
 *        Units chosen to match BME280's native compensated output format
 *        so the mock and real driver produce identical-shaped data.
 */

typedef struct 
{
    int32_t temperature_c_x100;
    uint32_t humidity_pct_x1024;
    uint32_t pressure_pa;
    uint32_t timestamp_ms;
}bme280_reading_t;

/**
 * @brief Acquire one sensor reading.
 * @param out  Pointer to struct to fill.
 * @return true if reading is valid, false on sensor failure.
 *
 * Implemented by EXACTLY ONE of:
 *   - sensor_mock.c   (Phase 5 — synthetic data, no hardware needed)
 *   - sensor_bme280.c (Phase 4-retrofit — real I2C driver, once hardware arrives)
*/

bool bme280_read(bme280_reading_t *out);

#endif /* SENSOR_IFACE_H */
