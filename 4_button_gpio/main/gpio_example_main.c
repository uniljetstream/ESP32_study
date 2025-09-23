#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_PIN 22
#define PUSH_BUTTON_PIN 23

void app_main(void)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);

    while (1)
    {
        if (gpio_get_level(PUSH_BUTTON_PIN) == 1)
        {
            gpio_set_level(LED_PIN, 1);
            ESP_LOGI("tag", "on");
        }
        else
        {
            gpio_set_level(LED_PIN, 0);
            ESP_LOGI("tag", "off");
        }

        vTaskDelay(50);
    }
}