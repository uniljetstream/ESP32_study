idf_component_register(SRCS "mcpwm_servo_control_example_main.c"
                       PRIV_REQUIRES esp_driver_ledc esp_adc
                       INCLUDE_DIRS ".")
