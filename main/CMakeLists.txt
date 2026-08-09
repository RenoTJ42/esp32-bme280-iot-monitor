idf_component_register(
    SRCS         "main.c"
    INCLUDE_DIRS "."
    REQUIRES     uart_driver
                 gpio_driver
                 sensor_iface
                 sensor_mock
                 tasks
                 esp_partition
                 log
                 esp_common
                 freertos
                 heap
)
