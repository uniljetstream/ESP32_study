#include "driver/i2c.h" //i2c 통신 드라이버
#include "esp_log.h"    //로그용 드라이버 3개
#include <stdio.h>
#include <string.h>

static const char *TAG = "i2c-master"; // information log를 사용하므로 TAG 설정

// i2c 마스터 클럭 핀
#define I2C_MASTER_SCL_IO 22
// i2c 마스터 데이터 핀
#define I2C_MASTER_SDA_IO 21
// i2c 마스터 클럭 주파수
#define I2C_MASTER_FREQ_HZ 100000
// i2c 마스터는 tx 버퍼가 필요없다. 비활성화
#define I2C_MASTER_TX_BUF_DISABLE 0
// i2c 마스터는 rx버퍼가 필요없다. 비활성화
#define I2C_MASTER_RX_BUF_DISABLE 0
// slave 의 주소
#define SLAVE_ADDRESS 0x0A

// i2c master writeb bit
#define WRITE_BIT I2C_MASTER_WRITE
// i2c makster read bit
#define READ_BIT I2C_MASTER_READ
// slave로 부터 온 ack를 확인하는 변수
#define ACK_CHECK_EN 0x1
// i2c 마스터는 slave로 부터온 ack를 확인하지 않음
#define ACK_CHECK_DIS 0x0
// i2c ack 값
#define ACK_VAL 0x0
// i2c nack 값
#define NACK_VAL 0x1

int i2c_master_port = 0;
// i2c 마스터 초기화 함수
static esp_err_t i2c_master_init(void)
{
    // 내부 풀업 저항 구성, i2c 클럭 속도 설정
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        //.clk_flags = 0    //옵셥: I2C_SCLK_SRC_FLAG 사용할 수 있다. i2c 소스클럭을 여기서 선택
    };
    esp_err_t err =
        i2c_param_config(i2c_master_port, &conf); // i2c port 초기화. 첫번째 인수 i2c 포트, i2c 구성에 대한 포인터
    if (err != ESP_OK)
    {
        return err;
    }
    // i2c 드라이버 설치(i2c 포트 번호, 모드(마스터/슬레이브), rx 버퍼 비활성화(slave에 적용되는 수신버퍼 크기), tx버퍼
    // 비활성화(slave에 전송되는 전송버퍼 크기), 인터럽트플래그-사용하지 않아서 0(intr_alloc_flag))
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// i2c 마스터가 슬레이브에게 보내는 함수
static esp_err_t i2c_master_send(const uint8_t message[], int len)
{
    ESP_LOGI(TAG, "Sending Message = %s", message);

    esp_err_t ret; // i2c_master_cmd_begin 실행 결과 저장
    /*
    i2c명령 링크를 만들고 초기화. i2c 명령 링크 핸들러를 반환한다.
    i2c 명령 링크를 타고 들어가면 void* 임. 이건 전송할 명령을 저장하는 큐로 동작됨.
    */
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    /*
    명령 링크를 만든 후 i2c_master_start를 호출하여 명령 링크를 매개 변수에 전달. 이러면 i2c 마스터가 시작신호를 생성함.
    */
    i2c_master_start(cmd);
    /*
    i2c 프로토콜의 규칙 : 통신 시작 후 첫 바이트는 8비트 주소 바이트 여야한다.
    첫바이트 구성 : [7비트 슬레이브 주소][1비트 R(1)/W(0) 플래그]
    즉, 어떤 슬레이브에게 데이터를 받을 준비 or 보낼 준비 해라라고 알려줌.

    마지막 변수는 슬레이브가 데이터를 정상적으로 수신했는 지 확인하는 ack 신호를 확인 할지 결정
    ack 활성화(1) : ack 확인 활성화 -> ack 없으면 에러
    ack 비활성화(0) : ack 확인 비활성화 -> ack 무시
    i2c_master_write_byte(i2c 명령 링크, 전송할 1바이트 데이터, ack 신호 확인 활성화여부(bool));
    */
    i2c_master_write_byte(cmd, SLAVE_ADDRESS << 1 | WRITE_BIT, ACK_CHECK_EN);
    /*
    통신 시작 후 첫 바이트 이후는 1바이트만 전송해야함.
    이후에는 버퍼 사이즈만큼 전송됨.
    이 때 i2c_master_write 사용
    i2c_master_write(i2c 명령 링크, 데이터가 담긴 버퍼, ack 신호 비활성화 여부)
    */
    i2c_master_write(cmd, message, len, ACK_CHECK_EN);
    /*
    i2c 통신 종료 신호를 전송. 즉, 점유하고 있던 버스를 해제하고 다른 디바이스가 사용가능하게한다.
    STOP 조건:
        SDA 라인이 LOW → HIGH로 변경 (SCL이 HIGH일 때)
        슬레이브에게 "통신 끝났어"라고 알림
        버스가 idle 상태로 돌아감
    생략하면 :
        버스가 계속 점유됨
        다른 I2C 통신 불가능
        슬레이브가 혼란 상태
    */
    i2c_master_stop(cmd);

    /*
    cmd에 저장된 명령이 실제로 전송되는 함수임.
    성공, 실패, 타임아웃, 잘못된파라미터 여부를 반환함. 이걸로 에러 핸들링
    i2c_master_cmd_begin(i2c포트번호, i2c명령핸들, 최대 대기 틱(기다릴 시))
    */
    ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd); // 명령 링크 해제
    return ret;               // 전송 상태 반환
}

void app_main(void)
{
    const uint8_t on_command[] = "LED_ON";
    const uint8_t off_command[] = "LED_OFF";
    /*
    에러 체크 매크로
    함수 반환값이 ESP_OK 가 아니면 프로그램을 중단시킴.
    */
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    while (1)
    {
        i2c_master_send(on_command, sizeof(on_command));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i2c_master_send(off_command, sizeof(off_command));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}