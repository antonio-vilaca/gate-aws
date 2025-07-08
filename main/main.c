#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <inttypes.h> // for PRIu32
#include <stdio.h>

static const char *TAG = "PORTAO";

#define PIN_LOCK_RELAY 12
#define PIN_BOOT_RELAY 14
#define PIN_SENSOR1 13
#define PIN_SENSOR2 19
#define PIN_LED_GREEN 33
#define PIN_LED_BLUE 32
#define PIN_LED_OPEN 18
#define PIN_LED_CLOSED 17
#define BUTTON_GPIO 34
#define M1_GPIO     27
#define M2_GPIO     26

typedef enum {
    STOPPED = 0,
    OPENING,
    CLOSING
} gate_state_t;

typedef struct {
    gate_state_t state;
    int open_cycles;
    int close_cycles;
    int last_button;
    int counter;
} gate_t;

void task_sensores(void *pvParameter) {
    gate_t *gate = (gate_t *)pvParameter;

    while (1) {
        int sensor1 = gpio_get_level(PIN_SENSOR1);
        gpio_set_level(PIN_LED_CLOSED, sensor1 == 0);

        int sensor2 = gpio_get_level(PIN_SENSOR2);
        gpio_set_level(PIN_LED_OPEN, sensor2 == 0);

        int button = gpio_get_level(BUTTON_GPIO);

        vTaskDelay(pdMS_TO_TICKS(100));

       // Detect falling edge (button pressed)
if (gate->last_button == 1 && button == 0) {
    if (gate->state == STOPPED) {
        gate->counter++;
        if (gate->counter % 2 == 1) {
            ESP_LOGI(TAG, "Opening gate");
            gate->state = OPENING;
            gpio_set_level(M1_GPIO, 1);
            gpio_set_level(M2_GPIO, 0);
        } else {
            ESP_LOGI(TAG, "Closing gate");
            gate->state = CLOSING;
            gpio_set_level(M1_GPIO, 0);
            gpio_set_level(M2_GPIO, 1);
        }
    } else {
        // Moving -> stop immediately
        gpio_set_level(M1_GPIO, 0);
        gpio_set_level(M2_GPIO, 0);
        ESP_LOGI(TAG, "Stopping gate (button press)");
        gate->state = STOPPED;
    }
}
gate->last_button = button;

        // Update LEDs based on gate state
        // Auto-stop by sensors
        if (gate->state == OPENING && sensor2 == 0) {
            gpio_set_level(M1_GPIO, 0);
            gpio_set_level(M2_GPIO, 0);
            gate->state = STOPPED;
            gate->open_cycles = 0;
            ESP_LOGI(TAG, "Gate fully open (sensor)");
        }

        if (gate->state == CLOSING && sensor1 == 0) {
            gpio_set_level(M1_GPIO, 0);
            gpio_set_level(M2_GPIO, 0);
            gate->state = STOPPED;
            gate->close_cycles = 0;
            ESP_LOGI(TAG, "Gate fully closed (sensor)");
        }

        // Safety timeout (5 seconds)
        if (gate->state == OPENING) {
            gate->open_cycles++;
            if (gate->open_cycles > 150) {
                gpio_set_level(M1_GPIO, 0);
                gpio_set_level(M2_GPIO, 0);
                gate->state = STOPPED;
                gate->open_cycles = 0;
                ESP_LOGI(TAG, "Timeout: stopping opening");
            }
        }

        if (gate->state == CLOSING) {
            gate->close_cycles++;
            if (gate->close_cycles > 150) {
                gpio_set_level(M1_GPIO, 0);
                gpio_set_level(M2_GPIO, 0);
                gate->state = STOPPED;
                gate->close_cycles = 0;
                ESP_LOGI(TAG, "Timeout: stopping closing");
            }
        }

        // Reset counters if gate is stopped
        if (gate->state == STOPPED) {
            gate->open_cycles = 0;
            gate->close_cycles = 0;
        }
    }
}

void app_main() {
    ESP_LOGI(TAG, "Startup.. Free memory: %" PRIu32 " bytes",
             esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Configure output GPIOs
    gpio_config_t io_conf_saida = {
        .pin_bit_mask = ((1ULL << PIN_LOCK_RELAY) | (1ULL << PIN_BOOT_RELAY) |
                         (1ULL << PIN_LED_GREEN) | (1ULL << PIN_LED_BLUE) |
                         (1ULL << PIN_LED_OPEN) | (1ULL << PIN_LED_CLOSED) |
                         (1ULL << M1_GPIO) | (1ULL << M2_GPIO)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_saida);

    // Configure input GPIOs with pull-up
    gpio_config_t io_conf_entrada = {
        .pin_bit_mask = ((1ULL << PIN_SENSOR1) | (1ULL << PIN_SENSOR2)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_entrada);

    // GPIO34 does not support internal pull-up, configure without it
    gpio_config_t io_conf_botao = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0, // external pull-up required
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_botao);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize gate object
    static gate_t gate = {STOPPED, 0, 0, 1, 0};

    // Create task with gate reference
    xTaskCreate(&task_sensores, "task_sensores", 2048, &gate, 5, NULL);
}
