#pragma once
#include <Arduino.h>

namespace buzzer {
  void begin(uint8_t pinBuzzer);
  void setLeak(bool leak);
  void tick(uint32_t onMs, uint32_t periodMs, uint32_t freqHz);
}