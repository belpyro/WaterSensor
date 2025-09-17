#pragma once
#include <Arduino.h>

// Пины/тайминги
static const uint8_t PIN_BUZZER    = D5; // GPIO14
static const uint8_t PIN_VREF      = D1; // GPIO5
static const uint8_t PIN_LEAK_TEST = D2; // GPIO4

static const uint32_t BUZZ_FREQ_HZ   = 2840;
static const uint32_t BUZZ_ON_MS     = 1000;
static const uint32_t BUZZ_PERIOD_MS = 5000;

static const int ADC_LEAK_THRESHOLD  = 50;

// Отладка/режимы
static const bool BATTERY_MODE        = true;
static const bool DEBUG_FORCE_LEAK    = false;
static const bool DEBUG_SHORT_SLEEP   = false;
static const uint32_t SLEEP_OK_SEC      = 3600;
static const uint32_t SLEEP_RETRY_SEC   = 60;
static const uint32_t DEBUG_SLEEP_OK_SEC   = 15;
static const uint32_t DEBUG_SLEEP_RETRY_SEC= 5;

// Идентификаторы
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

// MQTT по умолчанию (можно переопределять из config.json)
static const char* DEFAULT_MQTT_HOST = "192.168.1.103";
static const uint16_t DEFAULT_MQTT_PORT = 1883;
static const char* MQTT_CLIENT_ID = "WaterSensor";
static const char* MQTT_TOPIC_STATUS = "home/water/status";

// Хостнейм
static const char* HOSTNAME = "WaterSensor";