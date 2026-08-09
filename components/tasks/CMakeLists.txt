idf_component_register(
    SRCS         "sensor_task.c"
                 "mqtt_task.c"
                 "wdt_task.c"
    INCLUDE_DIRS "."
    REQUIRES     sensor_iface
                 sensor_mock
                 gpio_driver
                 log
                 esp_common
                 freertos
)
