#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"

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

    while (1) {
        int adc_value;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value);
        printf("ADC Value: %d", adc_value);
        printf("\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
// #include <stdio.h>
// #include <stdlib.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/adc.h"
// #include "esp_adc_cal.h" 

// static esp_adc_cal_characteristics_t adc1_chars;

// void app_main(void)
// {
//     esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

//     adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
//     adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);

//     while (1) 
//     {
//         int adc_value = adc1_get_raw(ADC1_CHANNEL_4);
//         printf("ADC Value: %d", adc_value);
//         printf("\n");
//         vTaskDelay(500/ portTICK_PERIOD_MS);
//     }
// }

