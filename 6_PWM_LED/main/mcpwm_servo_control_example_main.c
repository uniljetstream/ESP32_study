#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/ledc.h"

#define SAMPLE_CNT 32
static const adc1_channel_t adc_channel = ADC_CHANNEL_4;
#define LEDC_GPIO 27
static ledc_channel_config_t ledc_channel;

static void init_hw(void)
{
    adc1_config_width(ADC_WIDH_BIT_10);
    adc1_config_channel_atten(adc_channel, ADC_ATEN_DB_11);
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000;
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc
}

void app_main
{

}
