#include "button.hpp"
#include <LittleFS.h>
#include <constants.hpp>
#include <ESP8266WiFi.h>

namespace {
  uint8_t s_pin = 255;
  bool s_prev = false;
  bool s_held = false;
  uint32_t s_downMs = 0;
  uint32_t s_inhibitUntil = 0;
  uint32_t s_forceLeakUntil = 0;
  bool s_resetRequested = false;
  bool s_resetFull = false;
}

namespace button {
  bool testLeak = false;
  bool inhibitSleep = false;
  bool resetConfig = false;
  bool holdActive = false;   // true while button is currently pressed

  static ResetCb s_onReset = nullptr;

  void setResetCallback(ResetCb cb) { s_onReset = cb; }

  void requestReset(bool fullWifi) {
    s_resetRequested = true;
    s_resetFull = fullWifi;
  }

  void begin(uint8_t pin) {
    s_pin = pin;
    pinMode(s_pin, INPUT_PULLUP);
    s_prev = (digitalRead(s_pin) == LOW);
  }

  void tick() {
    if (s_pin == 255) return;
    uint32_t now = millis();
    bool pressed = (digitalRead(s_pin) == LOW);

    if (pressed && !s_prev) {
      // edge: pressed
      s_downMs = now;
      s_held = true;
      holdActive = true;
    }

    // While holding: trigger inhibit as soon as duration >= long threshold
    if (pressed && s_prev) {
      uint32_t dur = now - s_downMs;
      if (dur >= BTN_LONG_MS && !inhibitSleep) {
        s_inhibitUntil = now + SLEEP_INHIBIT_MS;
        inhibitSleep = true;
        Serial.println(F("[BTN] Long press (hold) → inhibit sleep"));
        tone(PIN_BUZZER, BUZZ_FREQ_HZ, 120);
      }
    }

    if (!pressed && s_prev) {
      // edge: released
      s_held = false;
      holdActive = false;
      uint32_t dur = now - s_downMs;
      if (dur >= 20000) {
        // Ultra long → request full reset (config + WiFi)
        Serial.println(F("[BTN] Ultra long press → full reset requested"));
        s_resetRequested = true;
        s_resetFull = true;
        tone(PIN_BUZZER, BUZZ_FREQ_HZ, 100);    
        delay(20);
        tone(PIN_BUZZER, BUZZ_FREQ_HZ, 100);
      } else if (dur >= BTN_VERY_LONG_MS) {
        // Very long → request config reset only
        Serial.println(F("[BTN] Very long press → config reset requested"));
        s_resetRequested = true;
        s_resetFull = false;
      } else if (dur >= BTN_LONG_MS) {
        // already set inhibit during hold
        Serial.println(F("[BTN] Long press → inhibit confirmed"));
      } else if (dur >= BTN_SHORT_MIN_MS) {
        s_forceLeakUntil = now + 2000;
        testLeak = true;
        Serial.println(F("[BTN] Short press → leak test"));
        tone(PIN_BUZZER, BUZZ_FREQ_HZ, 100);
      }
    }

    s_prev = pressed;

    if (now > s_forceLeakUntil) testLeak = false;
    if (now > s_inhibitUntil) inhibitSleep = false;

    if (s_resetRequested) {
      // invoke delegate first (if any)
      if (s_onReset) {
        s_onReset(s_resetFull);
      }
      // default behavior: remove app config, optionally wipe WiFi creds
      if (!LittleFS.begin()) LittleFS.begin();
      if (LittleFS.exists("/config.json")) LittleFS.remove("/config.json");
      if (s_resetFull) {
        WiFi.disconnect(true); // erase WiFi credentials from SDK flash
      }
      Serial.println(s_resetFull ? F("[BTN] Full reset executed") : F("[BTN] Config reset executed"));
      delay(300);
      ESP.restart();
    }
  }
}