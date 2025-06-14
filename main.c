#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "config.h"


#define PIN_LED_R  11
#define PIN_LED_G  12
#define PIN_LED_B  13
#define PIN_BUZZER 10
#define PIN_BTN_A  5
#define PIN_BTN_B  6

// Handles
TaskHandle_t ledTaskHandle = nullptr;
TaskHandle_t buzzerTaskHandle = nullptr;

void ledTask(void* params) {
    while (true) {
        gpio_put(PIN_LED_R, 1); gpio_put(PIN_LED_G, 0); gpio_put(PIN_LED_B, 0);
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_put(PIN_LED_R, 0); gpio_put(PIN_LED_G, 1); gpio_put(PIN_LED_B, 0);
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_put(PIN_LED_R, 0); gpio_put(PIN_LED_G, 0); gpio_put(PIN_LED_B, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void buzzerTask(void* params) {
    while (true) {
        gpio_put(PIN_BUZZER, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_put(PIN_BUZZER, 0);
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

void buttonMonitorTask(void* params) {
    bool ledSuspended = false;
    bool buzzerSuspended = false;

    while (true) {
        if (gpio_get(PIN_BTN_A) == 0) {
            if (!ledSuspended) {
                vTaskSuspend(ledTaskHandle);
                ledSuspended = true;
            } else {
                vTaskResume(ledTaskHandle);
                ledSuspended = false;
            }
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
        }

        if (gpio_get(PIN_BTN_B) == 0) {
            if (!buzzerSuspended) {
                vTaskSuspend(buzzerTaskHandle);
                buzzerSuspended = true;
            } else {
                vTaskResume(buzzerTaskHandle);
                buzzerSuspended = false;
            }
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main() {
    stdio_init_all();

    // Configurar GPIOs
    gpio_init(PIN_LED_R); gpio_set_dir(PIN_LED_R, GPIO_OUT);
    gpio_init(PIN_LED_G); gpio_set_dir(PIN_LED_G, GPIO_OUT);
    gpio_init(PIN_LED_B); gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_init(PIN_BUZZER); gpio_set_dir(PIN_BUZZER, GPIO_OUT);

    gpio_init(PIN_BTN_A); gpio_set_dir(PIN_BTN_A, GPIO_IN); gpio_pull_up(PIN_BTN_A);
    gpio_init(PIN_BTN_B); gpio_set_dir(PIN_BTN_B, GPIO_IN); gpio_pull_up(PIN_BTN_B);

    // Criar tarefas
    xTaskCreate(ledTask, "LED Task", 256, nullptr, 1, &ledTaskHandle);
    xTaskCreate(buzzerTask, "Buzzer Task", 256, nullptr, 1, &buzzerTaskHandle);
    xTaskCreate(buttonMonitorTask, "Button Monitor", 256, nullptr, 2, nullptr);

    vTaskStartScheduler();

    while (1); 
}
