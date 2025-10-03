#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;

void Demo_Task(void *arg)
{
    while (1)
    {
        printf("Demo_Task printing..\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Demo_Task2(void *arg)
{
    while (1)
    {
        printf("Demo_Task2 printing..\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
    xTaskCreatePinnedToCore(Demo_Task2, "Demo_Task", 4096, NULL, 10, &myTaskHandle2, 1);
    // xTaskCreate와 똑같음. 다른 마지막 변수는 0 또는 1번 코어를 사용하라는 의미
}
