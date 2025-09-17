#include <Arduino.h>
#include <buzzer.hpp>

static uint8_t s_pin;
static bool s_leak = false;
static bool s_buzzing = false;
static uint32_t s_epoch = 0;

namespace buzzer {
  void begin(uint8_t pinBuzzer) {
    s_pin = pinBuzzer;
    pinMode(s_pin, OUTPUT);
    noTone(s_pin);
    s_epoch = millis();
  }

  void setLeak(bool leak) {
    s_leak = leak;
    if (!s_leak && s_buzzing) { noTone(s_pin); s_buzzing = false; }
  }

  void tick(uint32_t onMs, uint32_t periodMs, uint32_t freqHz) {
    if (!s_leak) return;
    uint32_t now = millis();
    uint32_t phase = (now - s_epoch) % periodMs;
    if (phase < onMs) {
      if (!s_buzzing) { tone(s_pin, freqHz); s_buzzing = true; }
    } else if (s_buzzing) {
      noTone(s_pin); s_buzzing = false;
    }
  }
}