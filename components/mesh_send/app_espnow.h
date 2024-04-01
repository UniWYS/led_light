/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_now.h"

#define ESPNOW_DEBUG                     (0)
#define ESPNOW_PAYLOAD_HEAD_LEN          (5)
#define ESPNOW_QUEUE_SIZE                (50)
#define ESPNOW_DEVICE_NAME               "Plant"
#define ESPNOW_GROUP_ID                  "group_id"
#define ESPNOW_DISTRIBUTION_NETWORK      "distribution_network"

typedef enum espnow_msg_mode {
    ESPNOW_MSG_MODE_INVALID = 0,
    ESPNOW_MSG_MODE_CONTROL = 1,
    ESPNOW_MSG_MODE_RESET   = 2
} ESPNOW_MSG_MODE;

enum {
    APP_ESPNOW_DATA_BROADCAST,
    APP_ESPNOW_DATA_UNICAST,
    APP_ESPNOW_DATA_MAX,
};

typedef enum {
    SENSOR_TEMPERATURE = 0,
    SENSOR_HUMIDITY,
    // Add more sensor types as needed
    SENSOR_MAX_TYPES
} sensor_type_t;

typedef struct {
    sensor_type_t data_type;
    union {
        float temperature;
        float humidity;
        // Add more sensor value types as needed
    } payload;
} sensor_data_t;

/* User defined field of ESPNOW data in this example. */
typedef struct {
    uint32_t seq;                         //Magic number which is used to determine which device to send unicast ESPNOW data.
    uint8_t mesh_id;                      //Mesh ID of ESPNOW data
    sensor_data_t sensor_data;
} __attribute__((packed)) app_espnow_data_t;

esp_err_t message_init(void);
esp_err_t message_deinit(void);
void esp_now_send_device(float value, sensor_type_t data_type, bool seq_init);
