#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define LED_PIN 27
#define PUSH_BUTTON_PIN 33

TaskHandle_t ISR = NULL;

void IRAM_ATTR button_isr_handler(void *arg)    //IRAM 내부 램
{
    xTaskResumeFromISR(ISR);
}

void interrupt_task(void *arg)
{
    bool led_status = false;
    while(1)
    {
        vTaskSuspend(NULL);
        led_status = !led_status;
        gpio_set_level(LED_PIN, led_status);
        printf("Button pressed!\n");
    }
}

void app_main()
{
    //PUSH_BUTTON_PIN, LED_PIN을 gpio로 정정
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN);
    esp_rom_gpio_pad_select_gpio(LED_PIN);

    //PUSH_BUTTON_PIN을 input 모드로 설정
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    //LED_PIN을 output 모드로 설정
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    //PUSH_BUTTON_PIN의 인터럽트를 상승엣지로 설정.
    gpio_set_intr_type(PUSH_BUTTON_PIN, GPIO_INTR_POSEDGE);

    //이 함수의 매개변수가 0이면 레벨 1, 2, 3의 non-shared 인터럽트가 할당됨.(esp_intr_alloc.h의 133-137라인)
    //gpio isr을 설치.
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    //gipo 핀에 isr 핸들러를 설치
    //gpio_install_isr_service()를 사용한 이후에는 반드시 실행되어야함.
    gpio_isr_handler_add(PUSH_BUTTON_PIN, button_isr_handler, NULL);

    //테스크 실행.
    xTaskCreate(interrupt_task, "interrupt_task", 4096, NULL, 10, &ISR);
}
