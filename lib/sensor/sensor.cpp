#include <Arduino.h>
#include <sensor.hpp>

static uint8_t s_vref;

namespace sensor {
  void begin(uint8_t pinVref) {
    s_vref = pinVref;
    pinMode(s_vref, OUTPUT);
    digitalWrite(s_vref, HIGH);
  }

  bool readLeak(int adcThreshold, bool debugForce) {
    if (debugForce) return true;
    digitalWrite(s_vref, HIGH);
    delay(5);
    int raw = analogRead(A0);
    Serial.printf("[LEAK] ADC=%d\n", raw);
    return raw >= adcThreshold;
  }
}