idf_component_register(
        SRCS "vitowifi-esp.cpp"
        REQUIRES VitoWiFi esp_driver_uart
        INCLUDE_DIRS "."
)