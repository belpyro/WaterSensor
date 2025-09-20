#include <Arduino.h>
#include <sensor.hpp>

static uint8_t s_vref;

namespace sensor {
  void begin(uint8_t pinVref) {
    s_vref = pinVref;
    pinMode(s_vref, INPUT_PULLUP);
  }

  bool readLeak(bool debugForce) {
    if (debugForce) return true;
    if (digitalRead(s_vref) == HIGH) return false;
    Serial.printf("[LEAK] Leak detected");
    return true;
  }
}