#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "freertos/semphr.h"

#define ESP_INR_FLAG_DEFAULT 0
#define LED_PIN 27
#define PUSH_BUTTON_PIN 33

SemaphoreHandle_t xSemaphore = NULL;    //세마포 핸들을 NULL로 지정.

TaskHandle_t ISR = NULL;

TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;

void IRAM_ATTR button_isr_handler(void *arg)
{  
    //인터럽트에 대한 세마포 할당, ISR과 태스크를 동기화하는데 사용
    xSemaphoreGiveFromISR(xSemaphore, NULL);  
    xTaskResumeFromISR(ISR);
}

void interrupt_task(void* arg)
{
    bool led_status = false;
    while(1)
    {
        vTaskSuspend(NULL);
        led_status = !led_status;
        gpio_set_level(LED_PIN, led_status);
    }
}

void Demo_Task(void *arg)
{
    while(1)
    {
        printf("Message Sent! [%lu] \n", xTaskGetTickCount());
        xSemaphoreGive(xSemaphore);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void Demo_Task2(void *arg)
{
    while(1)
    {
        if(xSemaphoreTake(xSemaphore, portMAX_DELAY))
        {
            printf("Received Message [%lu] \n", xTaskGetTickCount());
        }
    }
}

void app_main()
{
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN);
    esp_rom_gpio_pad_select_gpio(LED_PIN);

    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_intr_type(PUSH_BUTTON_PIN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(ESP_INR_FLAG_DEFAULT);

    gpio_isr_handler_add(PUSH_BUTTON_PIN, button_isr_handler, NULL);

    xSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
    xTaskCreatePinnedToCore(Demo_Task2, "Demo_Task2", 4096, NULL, 10, &myTaskHandle2, 1);

    xTaskCreate(interrupt_task, "interrupt_task", 4096, NULL, 10, &ISR);
}