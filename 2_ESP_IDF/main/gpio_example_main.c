#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "DELAY"
#define LED_PIN 2

void app_main(void)
{
#if 0  //실습1. LOGD, LOGV를 사용하기 위해 menuconfig에서 활성화 필요
    ESP_LOGE("LOG", "This is an error");
    ESP_LOGW("LOG", "This is an warning");
    ESP_LOGI("LOG", "This is an info");
    ESP_LOGD("LOG", "This is a debug");
    ESP_LOGV("LOG", "This is a verbose");
#elif 0  //실습2. esp_log_level_set으로 태그에 어디까지 오류를 메시지를 표현할지 지정
    esp_log_level_set("LOG", ESP_LOG_INFO);

    ESP_LOGE("LOG", "Tiis is an error");
    ESP_LOGW("LOG", "Tiis is a warning");
    ESP_LOGI("LOG", "Tiis is an info");
    ESP_LOGD("LOG", "Tiis is a debug");
    ESP_LOGV("LOG", "Tiis is a verbose");

    ESP_LOGE("TAG", "Tiis is an error");
    ESP_LOGW("TAG", "Tiis is a warning");
    ESP_LOGI("TAG", "Tiis is an info");
    ESP_LOGD("TAG", "Tiis is a debug");
    ESP_LOGV("TAG", "Tiis is a verbose");
#elif 0 //실습3. 1초 한 번씩 INFO 메시지 띄우기
    int i = 0;
    while(1)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "in loop %d", i++);
    }
#elif 0 //실습4. 보드의 led가 1초마다 껏다가 켜짐.
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    int ON = 0;
    while(true)
    {
        ON = !ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
#elif 1 //실습5. 키보드 입력받기, 좀 느리다
    char character = 0;
    char str[100];
    memset(str, 0, sizeof(str));

    while(character != '\n')
    {
        character = getchar();
        if(character != 0xff)
        {
            str[strlen(str)] = character;
            printf("%c", character);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    printf("USER Entered: %s\n", str);
#endif
}