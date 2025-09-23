// #include "HD44780.h"
// #include "sdkconfig.h"
// #include <driver/i2c.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <stdio.h>

// #define LCD_ADDR 0x27
// #define SDA_PIN 21
// #define SCL_PIN 22
// #define LCD_COLS 16
// #define LCD_ROWS 2

// void LCD_DemoTask(void *param)
// {
//     char num[20];
//     while (true)
//     {
//         LCD_home();
//         LCD_clearScreen();
//         LCD_writeStr("Hello World");
//         vTaskDelay(3000 / portTICK_PERIOD_MS);
//         LCD_clearScreen();
//         LCD_writeStr("Count 0-10");
//         vTaskDelay(3000 / portTICK_PERIOD_MS);
//         LCD_clearScreen();
//         for(int i = 0;i<=10;i++)
//         {
//             LCD_setCursor(8, 1);
//             sprintf(num, "%d", i);
//             LCD_writeStr(num);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//     }
// }


// void app_main(void)
// {
//     LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);

//     // Wait for LCD initialization to complete
//     vTaskDelay(1000 / portTICK_PERIOD_MS);

//     xTaskCreate(&LCD_DemoTask, "Demo Task", 2048, NULL, 5, NULL);
// }

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "HD44780.h"

#define LCD_ADDR 0x27
#define SDA_PIN  21
#define SCL_PIN  22
#define LCD_COLS 16
#define LCD_ROWS 2

void LCD_DemoTask(void* param)
{
    char num[20];
    while (true) {
        LCD_home();
        LCD_clearScreen();
        LCD_writeStr("16x2 I2C LCD");
        vTaskDelay(3000 /  portTICK_PERIOD_MS);
        LCD_clearScreen();
        LCD_writeStr("Lets Count 0-10!");
        vTaskDelay(3000 /  portTICK_PERIOD_MS);
        LCD_clearScreen();
        for (int i = 0; i <= 10; i++) {
            LCD_setCursor(8, 2);
            sprintf(num, "%d", i);
            LCD_writeStr(num);
            vTaskDelay(1000 /  portTICK_PERIOD_MS);
        }
  
    }
}

void app_main(void)
{
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    xTaskCreate(&LCD_DemoTask, "Demo Task", 2048, NULL, 5, NULL);
    
}