#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;
QueueHandle_t queue;

void Demo_Task(void *arg)
{
    //txBuffer 생성, 큐에 담길 한 항목.
    char txBuffer[50];
    //큐 사이에 데이터를 주고 받을 수 있는 queue 생성
    //5 항목을 대기열에 저장 가능, 한 항목에 담길 데이터 사이즈
    queue = xQueueCreate(5, sizeof(txBuffer));
    if (queue == 0) //null을 반환하면 할당 실해
    {
        printf("Failed to create queue = %p\n", queue);
    }

    //txBuffer에 메시지를 담아 전송
    sprintf(txBuffer, "Hello from Demo_Task 1");
    //큐, 큐에 넣어질 항목의 포인터, 큐가 가득찼으면 기다려야할 최대 시간(여기서는 큐가 가득차면 호출이 즉시 반환됨.)
    xQueueSend(queue, (void *)txBuffer, (TickType_t)0);

    sprintf(txBuffer, "Hello from Demo_Task 2");
    xQueueSend(queue, (void *)txBuffer, (TickType_t)0);

    sprintf(txBuffer, "Hello from Demo_Task 3");
    xQueueSend(queue, (void *)txBuffer, (TickType_t)0);

    while (1)
    {
        //무한 대기
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Demo_Task2(void *arg)
{
    //Demo_Task2가 발신한 메시지를 수신
    char rxBuffer[50];
    while(1)
    {   //isr에서 사용불가능 함수, 큐로 전달된 데이터 수신
        //isr에서 사용하려면 xQueueReceiveFromISR 사용
        //큐, 수신할 데이터를 담을 변수, 큐가 비어 있을 경우 항목 수신을 기다릴 시간
        if(xQueueReceive(queue, &rxBuffer, (TickType_t)5))
        {
            printf("Received data from queue == %s\n", rxBuffer);
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
}

void app_main()
{
    //Demo_Task 생성
    xTaskCreate(Demo_Task, "Demo Task", 4096, NULL, 10, &myTaskHandle);
    //Demo_Task2 생성, 0,1 코어 중 1 코어 사용 지정
    xTaskCreatePinnedToCore(Demo_Task2, "Demo Task2", 4096, NULL, 10, &myTaskHandle2, 1);
}