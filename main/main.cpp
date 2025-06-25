##include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "GATE";

// Pinos do projeto (substituir se necessário)
#define PIN_RELE        GPIO_NUM_12
#define PIN_BOOT_RELE   GPIO_NUM_14
#define PIN_SENSOR_FECHADO GPIO_NUM_13
#define PIN_SENSOR_ABERTO  GPIO_NUM_19
#define PIN_LED_VERDE   GPIO_NUM_33
#define PIN_LED_AZUL    GPIO_NUM_32
#define PIN_LED_ABERTO  GPIO_NUM_18
#define PIN_LED_FECHADO GPIO_NUM_17

// Estados
bool estadoSensorFechado = false;
bool estadoSensorAberto = false;

// Delay entre ciclos (em ms)
#define DELAY_LOOP_MS 1000

void inicializar_gpio()
{
    gpio_config_t io_conf = {};

    // Saídas: relés e LEDs
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_RELE) | (1ULL << PIN_BOOT_RELE) |
                           (1ULL << PIN_LED_VERDE) | (1ULL << PIN_LED_AZUL) |
                           (1ULL << PIN_LED_ABERTO) | (1ULL << PIN_LED_FECHADO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // Entradas: sensores com pull-up
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_SENSOR_FECHADO) | (1ULL << PIN_SENSOR_ABERTO);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "GPIOs configurados.");
}

extern "C" void app_main(void)
{
    inicializar_gpio();

    ESP_LOGI(TAG, "Sistema iniciado.");

    while (1) {
        // Lê sensores
        estadoSensorFechado = !gpio_get_level(PIN_SENSOR_FECHADO);
        estadoSensorAberto  = !gpio_get_level(PIN_SENSOR_ABERTO);

        // Atualiza LEDs
        gpio_set_level(PIN_LED_FECHADO, estadoSensorFechado);
        gpio_set_level(PIN_LED_ABERTO, estadoSensorAberto);

        // Exemplo: imprime estados
        ESP_LOGI(TAG, "Sensor FECHADO: %s | ABERTO: %s",
                 estadoSensorFechado ? "ATIVADO" : "DESATIVADO",
                 estadoSensorAberto  ? "ATIVADO" : "DESATIVADO");

        vTaskDelay(pdMS_TO_TICKS(DELAY_LOOP_MS));
    }
}
