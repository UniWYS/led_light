/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <esp_log.h>
#include "esp_err.h"

#include <app_insights.h>
#include <esp_rmaker_mqtt.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_scenes.h>
#include <esp_rmaker_console.h>
#include <esp_rmaker_ota.h>

#include <esp_rmaker_common_events.h>
#include <cJSON.h>

#include "app_sensor.h"

#include "app_bridge.h"
#include "esp_mesh_lite.h"
#include "app_rainmaker_ota.h"
#include "app_espnow.h"

esp_rmaker_device_t *Device;s

esp_rmaker_device_t *Device_1;

static const char *TAG = "app_rainmaker";

// Allow for scheduling 
extern esp_err_t __real_esp_rmaker_handle_set_params(char *data, size_t data_len, esp_rmaker_req_src_t src);

// Allow resubscribe to MQTT if disconnected
static void esp_rmaker_app_set_params_callback(const char *topic, void *payload, size_t payload_len, void *priv_data);

/* Callback to handle param updates received from the RainMaker cloud */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);
    if (strcmp(param_name, "light power") == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", device_name, param_name);
        app_light_set_power(val.val.b);
    } 
    else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
    }
    else if (strcmp(param_name, ESP_RMAKER_DEF_HUE_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        app_light_set_hue(val.val.i);
    } 
    else if (strcmp(param_name, ESP_RMAKER_DEF_SATURATION_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        app_light_set_saturation(val.val.i);
    }
    else if (strcmp(param_name, "LED Mode") == 0) {
        ESP_LOGI("haha", "Received value = %s for %s - %s",
                val.val.s, device_name, param_name);
        app_light_set_mode(val.val.s);
    }
    else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}

/* Event handler for catching RainMaker events */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == RMAKER_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_INIT_DONE:
                ESP_LOGI(TAG, "RainMaker Initialised.");
                break;
            case RMAKER_EVENT_CLAIM_STARTED:
                ESP_LOGI(TAG, "RainMaker Claim Started.");
                break;
            case RMAKER_EVENT_CLAIM_SUCCESSFUL:
                ESP_LOGI(TAG, "RainMaker Claim Successful.");
                break;
            case RMAKER_EVENT_CLAIM_FAILED:
                ESP_LOGI(TAG, "RainMaker Claim Failed.");
                break;
            default:
                ESP_LOGW(TAG, "Unhandled RainMaker Event: %"PRId32"", event_id);
        }
    } else if (event_base == RMAKER_COMMON_EVENT) {
        switch (event_id) {
            case RMAKER_MQTT_EVENT_PUBLISHED: {
                break;
            }
            case RMAKER_EVENT_REBOOT:
                ESP_LOGI(TAG, "Rebooting in %d seconds.", *((uint8_t *)event_data));
                break;
            case RMAKER_EVENT_WIFI_RESET:
                ESP_LOGI(TAG, "Wi-Fi credentials reset.");
                break;
            case RMAKER_EVENT_FACTORY_RESET:
                ESP_LOGI(TAG, "Node reset to factory defaults.");
                esp_mesh_lite_erase_rtc_store();
                break;
            default:
                ESP_LOGW(TAG, "Unhandled RainMaker Common Event: %"PRId32"", event_id);
        }
    } else {
        ESP_LOGW(TAG, "Invalid event received!");
    }
}

void esp_rmaker_control_light_by_user(char* data)
{
    __real_esp_rmaker_handle_set_params((char *)data, strlen(data), ESP_RMAKER_REQ_SRC_CLOUD);
}

char Device_payload[ESPNOW_PAYLOAD_MAX_LEN];
esp_err_t __wrap_esp_rmaker_handle_set_params(char *data, size_t data_len, esp_rmaker_req_src_t src)
{
    ESP_LOGI(TAG, "Received params: %.*s", data_len, data);
    return ESP_OK;
}

static void esp_rmaker_app_set_params_callback(const char *topic, void *payload, size_t payload_len, void *priv_data)
{
    esp_rmaker_handle_set_params((char *)payload, payload_len, ESP_RMAKER_REQ_SRC_CLOUD);
}


void app_rainmaker_start(void)
{
    esp_rmaker_console_init();
    /* Initialize Application specific hardware drivers and
     * set initial state.
     */
    app_driver_init();

    /* Register an event handler to catch RainMaker events */
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_wifi_init() but before app_wifi_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "ESP Rainmaker Device", "Trial");
    if(!node){
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }
    esp_rmaker_node_add_attribute(node, "LiteMeshDevice", "1");

    // Create a category
    Device = esp_rmaker_device_create("Plant", "my.device.plant", NULL);
    esp_rmaker_device_add_cb(Device, write_cb, NULL);
    // Create a naming parameter to rename
    esp_rmaker_device_add_param(Device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, "Plant"));
    // Create data structure, naming and add
    esp_rmaker_param_t *power_param = esp_rmaker_param_create("light power", "esp.param.toggle", esp_rmaker_bool(DEFAULT_POWER), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type(power_param, ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param(Device, power_param);

    esp_rmaker_param_t *mode_param = esp_rmaker_param_create("LED Mode", "	esp.param.app-selector", esp_rmaker_str(DEFAULT_LED_MODE), PROP_FLAG_READ | PROP_FLAG_WRITE);
    static const char *valid_strs[] = {"Always","Blink","Breath", "Rainbow Wave"}; 
    esp_rmaker_param_add_valid_str_list(mode_param, valid_strs,4);
    esp_rmaker_param_add_ui_type(mode_param, ESP_RMAKER_UI_DROPDOWN);
    esp_rmaker_device_add_param(Device, mode_param);

    esp_rmaker_param_t *brightness_param = esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, DEFAULT_BRIGHTNESS);
    esp_rmaker_device_add_param(Device, brightness_param);
    esp_rmaker_param_t *hue_param = esp_rmaker_hue_param_create(ESP_RMAKER_DEF_HUE_NAME, DEFAULT_HUE);
    esp_rmaker_device_add_param(Device, hue_param);
    esp_rmaker_param_t *saturation_param = esp_rmaker_saturation_param_create(ESP_RMAKER_DEF_SATURATION_NAME, DEFAULT_SATURATION);
    esp_rmaker_device_add_param(Device, saturation_param);

    esp_rmaker_param_t *temp_param = esp_rmaker_temperature_param_create("Temperature", DEFAULT_TEMPERATURE);
    esp_rmaker_device_add_param(Device, temp_param);
    esp_rmaker_param_t *humd_param = esp_rmaker_temperature_param_create("Humidity", DEFAULT_HUMIDITY);
    esp_rmaker_device_add_param(Device, humd_param);
    esp_rmaker_param_t *soil_param = esp_rmaker_temperature_param_create("Soil moisture", DEFAULT_SOIL);
    esp_rmaker_device_add_param(Device, soil_param);
    esp_rmaker_param_t *ldr_param = esp_rmaker_temperature_param_create("Light Intensity", DEFAULT_LIGHT);
    esp_rmaker_device_add_param(Device, ldr_param);
    esp_rmaker_param_t *dist_param = esp_rmaker_param_create("Distance/m", NULL, esp_rmaker_float(DEFAULT_DISTANCE), PROP_FLAG_READ);
    esp_rmaker_device_add_param(Device, dist_param);

    esp_rmaker_device_assign_primary_param(Device, power_param);
    esp_rmaker_node_add_device(node, Device);


    Device_1 = esp_rmaker_device_create("Plant_from_peer", NULL, NULL);
    // Create a naming parameter to rename
    esp_rmaker_device_add_param(Device_1, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, "Collection from peer"));
    // Create data structure, naming and add
    esp_rmaker_param_t *temp1_param = esp_rmaker_temperature_param_create("Temperature", DEFAULT_TEMPERATURE);
    esp_rmaker_device_add_param(Device_1, temp1_param);
    esp_rmaker_param_t *humd1_param = esp_rmaker_temperature_param_create("Humidity", DEFAULT_TEMPERATURE);
    esp_rmaker_device_add_param(Device_1, humd1_param);
    esp_rmaker_node_add_device(node, Device_1);

    esp_rmaker_system_serv_config_t serv_config = {
        .flags = SYSTEM_SERV_FLAGS_ALL,
        .reset_reboot_seconds = 2,
    };

    esp_rmaker_system_service_enable(&serv_config);

    /* Enable timezone service which will be require for setting appropriate timezone
     * from the phone apps for scheduling to work correctly.
     * For more information on the various ways of setting timezone, please check
     * https://rainmaker.espressif.com/docs/time-service.html.
     */
    esp_rmaker_timezone_service_enable();

    /* Enable scheduling. */
    esp_rmaker_schedule_enable();

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    app_insights_enable();

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();
}
