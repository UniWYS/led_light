/* Mesh Internal Communication Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "esp_err.h"
#include "esp_mesh.h"
#include "app_mesh_light.h"
#include "driver/gpio.h"
#include "led_strip.h"

/*******************************************************
 *                Constants
 *******************************************************/
// RGB pin
#define RGB_GPIO 48

/*******************************************************
 *                Variable Definitions
 *******************************************************/
static bool s_light_inited = false;

static led_strip_handle_t led_strip;

/*******************************************************
 *                Function Definitions
 *******************************************************/
esp_err_t mesh_light_init(void)
{
    if (s_light_inited == true) {
        return ESP_OK;
    }
    s_light_inited = true;

    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);

    mesh_light_set(MESH_LIGHT_INIT);
    return ESP_OK;
}

esp_err_t mesh_light_set(int color)
{
    switch (color) {
    case MESH_LIGHT_PUR:
        /* Red */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 10, 2, 15);
        led_strip_refresh(led_strip);
        break;
    case MESH_LIGHT_GREEN:
        /* Green */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 0, 16, 0);
        led_strip_refresh(led_strip);
        break;
    case MESH_LIGHT_BLUE:
        /* Blue */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 0, 0, 16);
        led_strip_refresh(led_strip);
        break;
    case MESH_LIGHT_YELLOW:
        /* Yellow */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 16, 16, 0);
        led_strip_refresh(led_strip);
        break;
    case MESH_LIGHT_PINK:
        /* Pink */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 15, 5, 8);
        led_strip_refresh(led_strip);
        break;
    case MESH_LIGHT_INIT:
        /* can't say */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        led_strip_refresh(led_strip);
        break;
    case MESH_LIGHT_WARNING:
        /* warning */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 16, 0, 0);
        led_strip_refresh(led_strip);
        break;
    default:
        /* off */
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, 0, 0, 0, 0);
        led_strip_refresh(led_strip);
    }
    return ESP_OK;
}

void mesh_connected_indicator(int layer)
{
    switch (layer) {
    case 1:
        mesh_light_set(MESH_LIGHT_BLUE);
        break;
    case 2:
        mesh_light_set(MESH_LIGHT_YELLOW);
        break;
    case 3:
        mesh_light_set(MESH_LIGHT_PUR);
        break;
    case 4:
        mesh_light_set(MESH_LIGHT_PINK);
        break;
    case 5:
        mesh_light_set(MESH_LIGHT_GREEN);
        break;
    case 6:
        mesh_light_set(MESH_LIGHT_WARNING);
        break;
    default:
        mesh_light_set(0);
    }
}

void mesh_disconnected_indicator(void)
{
    mesh_light_set(MESH_LIGHT_WARNING);
}