#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_CNT 32
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_WIDTH ADC_BITWIDTH_12
#define LEDC_GPIO 27

static const char *TAG = "PWM_LED";
static adc_oneshot_unit_handle_t adc1_handle;
static ledc_channel_config_t ledc_channel;

static void init_hw(void)
{
    // ADC 초기화 (새로운 ADC oneshot API 사용)
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_WIDTH,
        .atten = ADC_ATTEN,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config);

    // LEDC 타이머 설정
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    // LEDC 채널 설정
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.duty = 0;
    ledc_channel.gpio_num = LEDC_GPIO;
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.hpoint = 0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel_config(&ledc_channel);
}

void app_main(void)
{
    ESP_LOGI(TAG, "PWM LED 제어 시작");
    init_hw();

    while (1)
    {
        uint32_t adc_val = 0;
        int adc_raw;

        // ADC 값 샘플링
        for (int i = 0; i < SAMPLE_CNT; ++i)
        {
            adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &adc_raw);
            adc_val += adc_raw;
        }
        adc_val /= SAMPLE_CNT;

        ESP_LOGI(TAG, "ADC 값: %lu", adc_val);

        // PWM duty cycle 설정 (12비트 해상도에 맞게 조정)
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, adc_val);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
