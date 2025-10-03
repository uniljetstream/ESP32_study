#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

TaskHandle_t myTaskHandle = NULL;

void Demo_Task(void *arg)
{
    while (1)
    {
        printf("Demo_Task printing..\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
    // task가 될 함수 이름, task 설명, task사이즈(워드 단위), 함수에 전달될 매개변수, 우서순위,  태스크 기능
    // 변경용(resume, delete, get 등)
}
