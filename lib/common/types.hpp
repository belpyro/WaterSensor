#pragma once
#include <Arduino.h>

struct AppConfig {
  char deviceName[32];
  char mqttServer[64];
  uint16_t mqttPort;
  bool alarmEnabled;
};