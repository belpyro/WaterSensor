#ifndef ESPWATERTEST_COMMON_CONSTANTS_HPP
#define ESPWATERTEST_COMMON_CONSTANTS_HPP
#include <Arduino.h>

// Pins and timings
static const uint8_t PIN_BUZZER    = D5; // GPIO14
static const uint8_t PIN_VREF      = D1; // GPIO5
static const uint8_t PIN_LEAK_TEST = D2; // GPIO4

static const uint32_t BUZZ_FREQ_HZ   = 2840;
static const uint32_t BUZZ_ON_MS     = 1000;
static const uint32_t BUZZ_PERIOD_MS = 5000;

// Thresholds and sleep
static const int  ADC_LEAK_THRESHOLD = 50;

// These macros can be overridden via build flags, e.g. -DBATTERY_MODE=0
#ifndef BATTERY_MODE
#define BATTERY_MODE 1
#endif
#ifndef DEBUG_FORCE_LEAK
#define DEBUG_FORCE_LEAK 0
#endif
#ifndef DEBUG_SHORT_SLEEP
#define DEBUG_SHORT_SLEEP 0
#endif
static const uint32_t SLEEP_OK_SEC         = 3600;
static const uint32_t SLEEP_RETRY_SEC      = 60;
static const uint32_t DEBUG_SLEEP_OK_SEC   = 15;
static const uint32_t DEBUG_SLEEP_RETRY_SEC= 5;

// Identifiers and defaults
#ifndef APP_VERSION
#define APP_VERSION "1.0.0"
#endif
#ifndef BASIC_AUTH_USER
#define BASIC_AUTH_USER "admin"
#endif
#ifndef BASIC_AUTH_PASS
#define BASIC_AUTH_PASS "admin123"
#endif
#ifndef MDNS_NAME
#define MDNS_NAME "esp8266"
#endif
#ifndef USE_WIFIMANAGER
#define USE_WIFIMANAGER 1
#endif

static const char* DEFAULT_MQTT_HOST = "192.168.1.103";
static const uint16_t DEFAULT_MQTT_PORT = 1883;
static const char* MQTT_CLIENT_ID = "WaterSensor";
static const char* MQTT_TOPIC_STATUS = "home/water/status";
static const char* MQTT_TOPIC_ACTIVE = "home/water/active";

static const char* HOSTNAME = "WaterSensor";

// ===== OTA config =====
#ifndef OTA_ENABLE
#define OTA_ENABLE 1
#endif
#ifndef WEB_OTA_ENABLE
#define WEB_OTA_ENABLE 1
#endif
#ifndef OTA_PASSWORD
#define OTA_PASSWORD "ota_password"
#endif
#ifndef OTA_PORT
#define OTA_PORT 8266
#endif

// ===== Test button behavior =====
#ifndef BTN_SHORT_MIN_MS
#define BTN_SHORT_MIN_MS 50
#endif
#ifndef BTN_LONG_MS
#define BTN_LONG_MS 3000
#endif
#ifndef BTN_VERY_LONG_MS
#define BTN_VERY_LONG_MS 10000
#endif
#ifndef SLEEP_INHIBIT_MS
#define SLEEP_INHIBIT_MS (10UL * 60UL * 1000UL) // 10 мин
#endif
#ifndef BOOT_GRACE_MS
#define BOOT_GRACE_MS 5000 // 5 с grace после старта
#endif

#endif // ESPWATERTEST_COMMON_CONSTANTS_HPP