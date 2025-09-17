#pragma once
#include <Arduino.h>

namespace sensor {
  void begin(uint8_t pinVref, uint8_t pinTest);
  bool readLeak(int adcThreshold, bool debugForce);
}