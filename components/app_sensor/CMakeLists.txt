idf_component_register(SRCS "app_mesh_light.c" "app_sensor.c"
                       INCLUDE_DIRS "."
                       REQUIRES app_reset rmaker_common esp_rainmaker ultrasonic mesh_send mesh_lite esp_adc)