#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "i2c-slave";

#define LED_PIN 2

// i2c 클럭 핀
#define I2C_SLAVE_SCL_IO 22
// i2c 데이터 핀
#define I2C_SLAVE_SDA_IO 21
// I2C 클럭
#define I2C_SLAVE_FREQ_HZ 100000
// slave tx 버퍼, 마스터는 필요 없지마 슬레이브는 필요
#define I2C_SLAVE_TX_BUF_LEN 255
// slave rx 버퍼, 마스터는 필요 없지마 슬레이브는 필요
#define I2C_SLAVE_RX_BUF_LEN 255
// i2c 슬레이브 주소
#define ESP_SLAVE_ADDR 0x0A

// i2c 마스터 쓰기
#define WRITE_BIT I2C_MASTER_WRITE
// i2c 마스터 읽기
#define READ_BIT I2C_MASTER_READ
// i2c 마스터가 slave로 부터 ack를 받아서 처리할 지 결정하는 변수
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
// ack의 값을 정하는 변수
#define ACK_VAL 0x0
#define NACK_VAL 0x1

// esp32는 보통 2개의 i2c 컨트롤러 보유. 어느 걸 쓸지 결정
int i2c_slave_port = 0;
static esp_err_t i2c_slave_init(void)
{
    i2c_config_t conf_slave = {     //i2c 포트 초기화 값 설정
        // 소프트웨어 i2c 지원. 원하는 핀 선택 가능
        //sda, scl 핀 선택. 자주 쓰이는 핀은 22, 21
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        //sda, scl에 풀업 저항 enable
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        //i2c slave 마스터 설정
        .mode = I2C_MODE_SLAVE,
        //7비트 주소 모드 사용(0 : 7bit, 1 : 10bit)
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = ESP_SLAVE_ADDR, // 원하는 주소 가능. slave 주소 설정
        .clk_flags = 0, //클럭관련 플래그(디포트 0)
    };
    esp_err_t err = i2c_param_config(i2c_slave_port, &conf_slave);  //설정된 매개변수 바탕으로 포트 초기화, (포트, 설정값의 포인터)
    if (err != ESP_OK)
    {
        return err;
    }
    /*
    i2c 드라이버 설치
    i2c 포트 번호, 모드 선택, rx 버퍼 사이즈(long unsinged int), tx 버퍼 사이즈, 인터럽트 플래그
    */ 
    return i2c_driver_install(i2c_slave_port, conf_slave.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}

void app_main()
{
    //가장 먼저 수신할 데이터 버퍼 생성
    uint8_t received_data[I2C_SLAVE_RX_BUF_LEN] = {0};
    //LED_PIN을 gpio로 구성
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    //i2c slave 초기화
    ESP_ERROR_CHECK(i2c_slave_init());
    ESP_LOGI(TAG, "I2C Slave initalized successfully");

    while (1)
    {
        //포트 번호, 내부 버퍼, rx 버퍼 사이즈, 대기 틱
        i2c_slave_read_buffer(i2c_slave_port, received_data, I2C_SLAVE_RX_BUF_LEN, 100 / portTICK_PERIOD_MS);
        //i2c로 받은 데이터가 임시로 저장되는 하드웨어 버퍼를 초기화 시킴. 하드웨어 버퍼는 i2c로 받은 데이터가 처음 도착하는 곳이다.
        i2c_reset_rx_fifo(i2c_slave_port);

        if (strcmp((const char *)received_data, "LED_ON") == 0)
        {
            ESP_LOGI(TAG, "Data Recived = %s", received_data);
            gpio_set_level(LED_PIN, 1);
        }
        else if (strncmp((const char *)received_data, "LED_OFF", 7) == 0)
        {
            ESP_LOGI(TAG, "Data Recived = %s", received_data);
            gpio_set_level(LED_PIN, 0);
        }
        //버퍼를 0으로 초기화
        memset(received_data, 0, I2C_SLAVE_RX_BUF_LEN); // 소프트웨어 i2c 드라이버 작업 공간 초기화.
    }
}
