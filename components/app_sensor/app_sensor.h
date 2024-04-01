/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_POWER       true
#define DEFAULT_HUE         180
#define DEFAULT_SATURATION  100
#define DEFAULT_BRIGHTNESS  25
#define DEFAULT_TEMPERATURE 25.0
#define DEFAULT_DISTANCE 0.0
#define DEFAULT_HUMIDITY 50.0
#define DEFAULT_SOIL 0.0
#define DEFAULT_LIGHT 0.0
#define SOIL_DRY 3200
#define SOIL_WET 1400
#define LDR_DARK_THRESHOLD 2000
#define REPORTING_PERIOD    60 /* Seconds */
#define DEFAULT_LED_MODE "Always"


void app_driver_init(void);
esp_err_t app_light_set_power(bool power);
esp_err_t app_light_set_brightness(uint16_t brightness);
esp_err_t app_light_set_hue(uint16_t hue);
esp_err_t app_light_set_saturation(uint16_t saturation);
esp_err_t app_light_set_mode(char *mode);

