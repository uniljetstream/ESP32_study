#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

TaskHandle_t myTaskHandle = NULL;

void Demo_Task(void* arg)
{
    while(1)
    {
        printf("Demo_Task printing..\n");
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
}
