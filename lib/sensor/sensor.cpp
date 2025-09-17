#include <Arduino.h>
#include <sensor.hpp>

static uint8_t s_vref, s_test;

namespace sensor {
  void begin(uint8_t pinVref, uint8_t pinTest) {
    s_vref = pinVref; s_test = pinTest;
    pinMode(s_vref, OUTPUT);
    digitalWrite(s_vref, HIGH);
    pinMode(s_test, INPUT_PULLUP);
  }

  bool readLeak(int adcThreshold, bool debugForce) {
    if (digitalRead(s_test) == LOW) {
      Serial.println("[TEST] PIN_LEAK_TEST=LOW → форсируем протечку");
      return true;
    }
    if (debugForce) return true;
    digitalWrite(s_vref, HIGH);
    delay(5);
    int raw = analogRead(A0);
    Serial.printf("[LEAK] ADC=%d\n", raw);
    return raw >= adcThreshold;
  }
}