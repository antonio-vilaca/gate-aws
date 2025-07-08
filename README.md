ESP-IDF template app
====================

This is a template application to be used with [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).

Please check [ESP-IDF docs](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for getting started instructions.

*Code in this repository is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.*


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "esp_netif.h"
#include "esp_event.h"

#include "example_connect.h"  // ajuste se não tiver isso

static const char *TAG = "PORTAO";


// ------------------- Pin Definitions ------------------- //
#define PIN_LOCK_RELAY    12
#define PIN_BOOT_RELAY    14
#define PIN_SENSOR1       13
#define PIN_SENSOR2       19
#define PIN_LED_GREEN     33
#define PIN_LED_BLUE      32
#define PIN_LED_OPEN      18
#define PIN_LED_CLOSED    17
//-------------------------------------------------------//

void task_sensores(void *pvParameter)
{
    static int estado_ant = -1; // Valor inicial diferente para forçar atualização

    while (1) {
        // Sensor 1: gate closed
        int sensor1 = gpio_get_level(PIN_SENSOR1);
        if(sensor1 == 0) {
            gpio_set_level(PIN_LED_CLOSED, 1);
        } else {
            gpio_set_level(PIN_LED_CLOSED, 0);
        }

        // Sensor 2: gate opened
        int sensor2 = gpio_get_level(PIN_SENSOR2);
        if(sensor2 == 0) {
            gpio_set_level(PIN_LED_OPEN, 1);
        } else {
            gpio_set_level(PIN_LED_OPEN, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main()

{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %"PRIu32" bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);

    // Inicializa NVS partition
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

   // Configurar saídas (relés e LEDs)
    gpio_config_t io_conf_saida = {
        .pin_bit_mask = (
            (1ULL << PIN_LOCK_RELAY) |
            (1ULL << PIN_BOOT_RELAY) |
            (1ULL << PIN_LED_GREEN)  |
            (1ULL << PIN_LED_BLUE)   |
            (1ULL << PIN_LED_OPEN)   |
            (1ULL << PIN_LED_CLOSED)
        ),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_saida);
    gpio_set_level(PIN_LED_GREEN, 0); // starts off

    // Configurar entradas (sensores)
    gpio_config_t io_conf_entrada = {
        .pin_bit_mask = ( (1ULL << PIN_SENSOR1) | (1ULL << PIN_SENSOR2) ),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,      // Ative o pull-up se seus sensores forem contatos secos (tipo reed switch)
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_entrada);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Configura Wi-Fi/Ethernet
    ESP_ERROR_CHECK(example_connect());


    xTaskCreate(&task_sensores, "task_sensores", 2048, NULL, 5, NULL);
    xTaskCreate(&task_publica_status, "task_publica_status", 2048, NULL, 5, NULL);
}


