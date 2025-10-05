#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>    //FreeRTOS semaphore 라이브러리
#include <freertos/task.h>
#include <stdio.h>

SemaphoreHandle_t xSemaphore = NULL;    //세마포 핸들

TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;

/*
이 코드는 Demo_Task에서 Demo_Task2 로 세마포를 전송함.
*/
void Demo_Task(void *arg)
{
    while (1)
    {
        printf("Message Sent! [%lu] \n", xTaskGetTickCount());  //애플리케이션이 시작된 이후 경과된 시간을 같이 표시
        xSemaphoreGive(xSemaphore); //세마포 전달을 위해 사용하는 함수
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Demo_Task2(void *arg)
{
    while (1)
    {
        /*
        xSemaphoreTake()는 세마포를 마져오는 함수 이다.
        * @arg 세마포 핸들, 세마포가 사용가능해 질때가지 대기 상태에서 기다릴 시간
        */
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY))
        {
            //세마포가 수신을 완료했으면 나옴.
            printf("Received Message [%lu] \n", xTaskGetTickCount());
        }
    }
}

void app_main()
{   
    //이진 세마포 생성
    xSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
    xTaskCreatePinnedToCore(Demo_Task2, "Demo_Task2", 4096, NULL, 10, &myTaskHandle2, 1);
}