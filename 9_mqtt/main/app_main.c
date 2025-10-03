/* MQTT (over TCP) 예제

   이 예제 코드는 퍼블릭 도메인(또는 선택적으로 CC0 라이센스)입니다.

   법률이 요구하는 경우를 제외하고, 이 소프트웨어는 명시적이든 묵시적이든
   어떠한 종류의 보증이나 조건 없이 "있는 그대로" 제공됩니다.
*/

// 표준 C 라이브러리
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// ESP-IDF 시스템 헤더
#include "esp_system.h"      // ESP32 시스템 함수 (메모리 관리 등)
#include "nvs_flash.h"       // NVS(Non-Volatile Storage) 플래시 메모리
#include "esp_event.h"       // 이벤트 루프 시스템
#include "esp_netif.h"       // 네트워크 인터페이스 추상화 계층
#include "protocol_examples_common.h"  // Wi-Fi/Ethernet 연결 헬퍼 함수

// 로깅 및 MQTT 라이브러리
#include "esp_log.h"         // ESP32 로그 시스템
#include "mqtt_client.h"     // MQTT 클라이언트 라이브러리

// 로그 태그 정의 (로그 출력 시 [mqtt_example] 형태로 표시됨)
static const char *TAG = "mqtt_example";


/**
 * @brief 에러 코드가 0이 아닐 때만 에러 로그를 출력하는 유틸리티 함수
 *
 * @param message 에러 메시지 설명
 * @param error_code 검사할 에러 코드
 */
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief MQTT 이벤트를 수신하기 위해 등록된 이벤트 핸들러
 *
 *  이 함수는 MQTT 클라이언트 이벤트 루프에 의해 호출됩니다.
 *
 * @param handler_args 이벤트에 등록된 사용자 데이터
 * @param base 핸들러의 이벤트 베이스(이 예제에서는 항상 MQTT 베이스)
 * @param event_id 수신된 이벤트의 ID
 * @param event_data 이벤트 데이터, esp_mqtt_event_handle_t
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // 이벤트 루프에서 디스패치된 이벤트 정보 로깅 (디버그 레벨)
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);

    // 이벤트 데이터를 MQTT 이벤트 핸들로 캐스팅
    esp_mqtt_event_handle_t event = event_data;
    // 이벤트에서 MQTT 클라이언트 핸들 추출
    esp_mqtt_client_handle_t client = event->client;
    // MQTT 작업(publish/subscribe)의 메시지 ID를 저장할 변수
    int msg_id;

    // 이벤트 ID에 따라 적절한 처리 수행
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:  // MQTT 브로커 연결 성공 시
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        // 1. "/topic/qos1"에 "data_3" 메시지 발행
        // 파라미터: (클라이언트, 토픽, 데이터, 길이(0=자동), QoS레벨, retain플래그)
        // QoS 1 = 최소 1번 전달 보장
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        // 2. "/topic/qos0" 토픽 구독 (QoS 0 = 최대 1번 전달, 보장 없음)
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // 3. "/topic/qos1" 토픽 구독 (QoS 1 = 최소 1번 전달 보장)
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // 4. "/topic/qos1" 구독 해제 (테스트 목적)
        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:  // MQTT 브로커와 연결이 끊어진 경우
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:  // 토픽 구독이 성공적으로 완료된 경우
        // event->msg_id: 구독 요청 시 받은 메시지 ID
        // event->data: 브로커가 반환한 QoS 레벨 (구독 성공 코드)
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ", event->msg_id, (uint8_t)*event->data);

        // 구독 성공 후 "/topic/qos0"에 테스트 메시지 발행 (QoS 0)
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:  // 토픽 구독 해제가 완료된 경우
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:  // 메시지 발행이 완료된 경우 (QoS 1,2에서만 확인됨)
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:  // 구독한 토픽으로부터 데이터를 수신한 경우
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        // %.*s 포맷: 길이를 지정한 문자열 출력 (null-terminator 없어도 안전)
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:  // MQTT 통신 중 에러 발생 시
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");

        // TCP/TLS 전송 계층 에러인 경우 상세 분석
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            // ESP-TLS 라이브러리에서 발생한 에러 체크
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            // TLS 스택(mbedTLS 등)에서 발생한 에러 체크
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            // 전송 소켓의 errno 체크
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            // errno를 사람이 읽을 수 있는 문자열로 출력
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:  // 정의되지 않은 기타 이벤트
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

/**
 * @brief MQTT 애플리케이션을 초기화하고 시작하는 함수
 *
 * MQTT 클라이언트를 설정하고, 브로커에 연결하며, 이벤트 핸들러를 등록합니다.
 */
static void mqtt_app_start(void)
{
    // MQTT 클라이언트 설정 구조체 초기화
    // CONFIG_BROKER_URL은 menuconfig에서 설정 (예: mqtt://broker.hivemq.com)
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };

#if CONFIG_BROKER_URL_FROM_STDIN
    // CONFIG_BROKER_URL_FROM_STDIN이 활성화된 경우 (Kconfig.projbuild에서 정의)
    // 런타임에 사용자로부터 브로커 URL을 입력받음
    char line[128];  // URL 저장 버퍼 (최대 128자)

    // 브로커 URL이 "FROM_STDIN"으로 설정되어 있는지 확인
    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");

        // 표준 입력에서 한 문자씩 읽어서 URL 구성
        while (count < 128) {
            int c = fgetc(stdin);  // 한 문자 읽기
            if (c == '\n') {       // 엔터 키 입력 시 종료
                line[count] = '\0';  // null-terminator 추가
                break;
            } else if (c > 0 && c < 127) {  // 유효한 ASCII 문자만 저장
                line[count] = c;
                ++count;
            }
            // CPU 점유율을 낮추기 위해 10ms 대기
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;  // 입력받은 URL로 설정
        printf("Broker url: %s\n", line);
    } else {
        // URL이 "FROM_STDIN"이 아닌 경우 설정 오류
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();  // 프로그램 종료
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    // MQTT 클라이언트 초기화 (설정 구조체 기반)
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    // 이벤트 핸들러 등록
    // ESP_EVENT_ANY_ID: 모든 MQTT 이벤트를 mqtt_event_handler로 처리
    // 마지막 NULL: 핸들러에 전달할 사용자 데이터 (없음)
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // MQTT 클라이언트 시작 (백그라운드에서 브로커 연결 시작)
    esp_mqtt_client_start(client);
}

/**
 * @brief 애플리케이션 메인 함수 (ESP32의 진입점)
 *
 * ESP32 부팅 시 자동으로 호출되며, 시스템 초기화 및 MQTT 클라이언트를 시작합니다.
 */
void app_main(void)
{
    // 시작 정보 로깅
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // 로그 레벨 설정 (디버깅을 위해 MQTT 관련 모듈은 VERBOSE로 설정)
    esp_log_level_set("*", ESP_LOG_INFO);                  // 모든 모듈: INFO 레벨
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);     // MQTT 클라이언트: 상세 로그
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);    // 이 예제: 상세 로그
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);  // 전송 계층: 상세 로그
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);         // TLS 암호화: 상세 로그
    esp_log_level_set("transport", ESP_LOG_VERBOSE);       // 전송 모듈: 상세 로그
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);          // MQTT outbox: 상세 로그

    // NVS(Non-Volatile Storage) 플래시 초기화
    // Wi-Fi 설정 등을 저장하는 데 필요
    ESP_ERROR_CHECK(nvs_flash_init());

    // TCP/IP 네트워크 인터페이스 초기화
    ESP_ERROR_CHECK(esp_netif_init());

    // 기본 이벤트 루프 생성 (Wi-Fi, MQTT 등의 이벤트 처리용)
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Wi-Fi 또는 Ethernet 연결 설정
    // menuconfig에서 설정한 대로 네트워크에 연결
    // 자세한 내용은 examples/protocols/README.md 참조
    ESP_ERROR_CHECK(example_connect());

    // MQTT 애플리케이션 시작 (브로커 연결 및 통신 시작)
    mqtt_app_start();
}
