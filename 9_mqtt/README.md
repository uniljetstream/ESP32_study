# ESP32 센서 데이터 MQTT 전송 시스템

ESP32에서 센서 데이터를 읽어 MQTT를 통해 Jetson으로 전송하는 시스템입니다.

## 프로젝트 구조

```
9_mqtt/main/
├── config.h              # 모든 설정 (Wi-Fi, MQTT, 토픽, 주기)
├── wifi_handler.h/c      # Wi-Fi 연결 관리
├── mqtt_handler.h/c      # MQTT 통신 관리
├── sensor_task.h/c       # 센서 읽기 및 전송
├── app_main.c            # 메인 파일
└── CMakeLists.txt        # 빌드 설정
```

## 주요 기능

- ✅ 자동 Wi-Fi 연결 및 재연결
- ✅ MQTT 브로커 자동 연결
- ✅ 주기적 센서 데이터 발행 (기본 5초)
- ✅ MQTT 명령으로 전송 주기 동적 변경
- ✅ JSON 형식 데이터 전송
- ✅ 양방향 통신 (ESP32 ↔ Jetson)

---

## 1단계: 설정 변경

### config.h 파일 수정

```c
// Wi-Fi 설정
#define WIFI_SSID "YourWiFiName"           // 본인 Wi-Fi 이름
#define WIFI_PASSWORD "YourWiFiPassword"   // 본인 Wi-Fi 비밀번호

// MQTT 브로커 설정 (Jetson IP)
#define MQTT_BROKER_URL "mqtt://192.168.x.x:1883"  // Jetson IP 주소

// 센서 데이터 전송 주기
#define DEFAULT_PUBLISH_INTERVAL_MS 5000  // 5초 (원하는 값으로 변경 가능)
```

---

## 2단계: Jetson에 MQTT 브로커 설치

### Mosquitto 설치
```bash
# 패키지 업데이트
sudo apt update

# Mosquitto 브로커 및 클라이언트 도구 설치
sudo apt install mosquitto mosquitto-clients -y

# Mosquitto 서비스 시작
sudo systemctl start mosquitto

# 부팅 시 자동 시작 설정
sudo systemctl enable mosquitto

# 실행 상태 확인
sudo systemctl status mosquitto
```

### Jetson IP 주소 확인
```bash
hostname -I
# 예시 출력: 192.168.0.100
```

이 IP를 config.h의 `MQTT_BROKER_URL`에 설정하세요.

---

## 3단계: ESP32 빌드 및 플래시

```bash
cd /home/ubuntu07/workingspace/esp-idf/9_mqtt

# 빌드
idf.py build

# 플래시 및 모니터
idf.py flash monitor
```

---

## 4단계: Jetson에서 MQTT 테스트

### 터미널 1: 센서 데이터 모니터링
```bash
mosquitto_sub -h localhost -t "esp32/sensor/data" -v
```

**출력 예시:**
```
esp32/sensor/data {"sensor":"temperature","value":25.43,"unit":"C","timestamp":12345}
esp32/sensor/data {"sensor":"temperature","value":26.12,"unit":"C","timestamp":12350}
```

### 터미널 2: ESP32에 명령 보내기

**전송 주기 변경:**
```bash
# 1초로 변경
mosquitto_pub -h localhost -t "esp32/command" -m "INTERVAL:1000"

# 2초로 변경
mosquitto_pub -h localhost -t "esp32/command" -m "INTERVAL:2000"

# 10초로 변경
mosquitto_pub -h localhost -t "esp32/command" -m "INTERVAL:10000"
```

### 터미널 3: ESP32 응답 확인
```bash
mosquitto_sub -h localhost -t "esp32/response" -v
```

**출력 예시:**
```
esp32/response {"status":"ok","interval":2000}
```

### 모든 MQTT 메시지 모니터링 (디버깅용)
```bash
mosquitto_sub -h localhost -t "#" -v
```
- `#`: 모든 토픽 구독 (와일드카드)
- ESP32와 Jetson 간 모든 통신을 볼 수 있음

---

## MQTT 토픽 구조

| 토픽 | 방향 | 설명 | 데이터 형식 |
|------|------|------|-------------|
| `esp32/sensor/data` | ESP32 → Jetson | 센서 데이터 발행 | JSON |
| `esp32/command` | Jetson → ESP32 | 명령 전송 | 문자열 |
| `esp32/response` | ESP32 → Jetson | 명령 응답 | JSON |

### 데이터 형식

**센서 데이터 (esp32/sensor/data):**
```json
{
  "sensor": "temperature",
  "value": 25.43,
  "unit": "C",
  "timestamp": 12345
}
```

**명령 (esp32/command):**
```
INTERVAL:3000
```

**응답 (esp32/response):**
```json
{
  "status": "ok",
  "interval": 3000
}
```

---

## 센서 연동 방법

### sensor_task.c 파일 수정

현재는 더미 데이터를 생성합니다:
```c
bool sensor_read_data(float *sensor_value)
{
    // 현재는 더미 데이터 (0~100 사이 랜덤 값)
    *sensor_value = (float)(esp_random() % 10000) / 100.0f;
    return true;
}
```

**실제 센서로 변경 예시 (DHT11 온도 센서):**
```c
bool sensor_read_data(float *sensor_value)
{
    // DHT11 센서 읽기
    float temperature = dht_read_temperature();

    if (temperature == -1) {
        return false;  // 읽기 실패
    }

    *sensor_value = temperature;
    return true;
}
```

---

## 시스템 동작 흐름

```
ESP32 부팅
  ↓
Wi-Fi 연결 (자동)
  ↓
MQTT 브로커 연결 (자동)
  ↓
"esp32/command" 토픽 구독 (자동)
  ↓
센서 태스크 시작
  ↓
주기적으로 반복:
  1. 센서 데이터 읽기
  2. JSON 생성
  3. MQTT로 발행
  4. 설정된 주기만큼 대기
```

---

## 전송 주기 변경 방법

### 방법 1: MQTT 명령으로 변경 (실시간, 추천)
```bash
mosquitto_pub -h localhost -t "esp32/command" -m "INTERVAL:2000"
```

### 방법 2: 코드에서 기본값 변경
config.h:
```c
#define DEFAULT_PUBLISH_INTERVAL_MS 3000  // 3초로 변경
```

### 방법 3: 코드에서 직접 호출
```c
void app_main(void)
{
    // 초기화...

    // 전송 주기를 2초로 변경
    sensor_set_publish_interval(2000);
}
```

---

## 문제 해결

### Wi-Fi 연결 실패
1. config.h에서 SSID와 비밀번호 확인
2. ESP32와 Jetson이 같은 Wi-Fi 네트워크에 있는지 확인
3. Wi-Fi 신호 강도 확인

### MQTT 연결 실패
1. Jetson에서 Mosquitto가 실행 중인지 확인:
   ```bash
   sudo systemctl status mosquitto
   ```
2. 방화벽 확인:
   ```bash
   sudo ufw allow 1883
   ```
3. Jetson IP 주소가 정확한지 확인

### 센서 데이터가 전송되지 않음
1. ESP32 시리얼 모니터에서 로그 확인:
   ```bash
   idf.py monitor
   ```
2. MQTT 연결 상태 확인
3. 전송 주기가 너무 길게 설정되지 않았는지 확인

---

## 참고 자료

- [ESP-IDF 공식 문서](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [MQTT 프로토콜](https://mqtt.org/)
- [Eclipse Mosquitto](https://mosquitto.org/)

---

## 라이선스

이 프로젝트는 퍼블릭 도메인입니다.
