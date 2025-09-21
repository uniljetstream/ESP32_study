#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_mac.h"

void app_main(void)
{
    // ADC oneshot 핸들 생성
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    // ADC 채널 설정
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);

    // 캘리브레이션 설정 (line fitting 사용)
    adc_cali_handle_t adc1_cali_handle = NULL;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &adc1_cali_handle);

    while (1) {
        int adc_value;
        int voltage;
        
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value);
        adc_cali_raw_to_voltage(adc1_cali_handle, adc_value, &voltage);
        
        printf("Voltage: %d mV", voltage);
        printf("\n");
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}