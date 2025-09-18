#include <Arduino.h>
#include <constants.hpp>
#include <types.hpp>
#include <config_fs.hpp>
#include <wifi.hpp>
#include <mdns.hpp>
#include <web.hpp>
#include <sensor.hpp>
#include <buzzer.hpp>
#include <mqtt_cli.hpp>
#include <power.hpp>
#include "ota.hpp"
#include "button.hpp"
#include "capcal.hpp"

#define WM_DEBUG_LEVEL

static AppConfig g_cfg;
static bool g_leak = false;
static bool g_isConnected = false;
static uint32_t g_bootMs = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("Booting v%s\n", APP_VERSION);

  // pinMode(PIN_VREF, OUTPUT);
  // digitalWrite(PIN_VREF, HIGH);

  cfgfs::beginFS();
  cfgfs::load(g_cfg);

  g_isConnected = wifiu::setupWiFi();
  mdnsu::setup();
  ota::begin();
  web::begin(&g_cfg);
  cap::begin(PIN_VREF);

  // sensor::begin(PIN_VREF);
  button::begin(PIN_LEAK_TEST);
  buzzer::begin(PIN_BUZZER);

  // Публикация статуса в MQTT один раз при старте
  mqt::configure(g_cfg);
  (void)mqt::publishActive();

  Serial.printf("Open http://%s.local/ or http://%s/\n", MDNS_NAME, WiFi.localIP().toString().c_str());

  if (DEBUG_SHORT_SLEEP){
    tone(PIN_BUZZER, BUZZ_FREQ_HZ, 10);
  }  
}

void loop() {
  web::loop();
  delay(5);
  
  button::tick();
  // Initialize grace window at first loop iteration (not in setup),
// so Wi-Fi connect time does not consume BOOT_GRACE_MS.
  static bool graceInit = false;
  if (!graceInit) {
    g_bootMs = millis();
    graceInit = true;    
  }  
  
  cap::tick();

  static uint32_t lastSense = 0;
  static bool sensorLeak = false;
  uint32_t now = millis();
  if (now - lastSense > 3000) {
    lastSense = now;
    sensorLeak = cap::isWet();//sensor::readLeak(ADC_LEAK_THRESHOLD, DEBUG_FORCE_LEAK);
  }
  bool leakNow = sensorLeak || button::testLeak;

  // Apply immediate changes to leak state
  if (leakNow != g_leak) {
    g_leak = leakNow;
    buzzer::setLeak(g_leak);
    (void)mqt::publishStatus(g_leak);
  }

  // Решение о сне
  static uint32_t lastSleepCheck = 0;
  if (BATTERY_MODE) {
    uint32_t now2 = millis();
    if (now2 - lastSleepCheck > 500) {
      lastSleepCheck = now2;
      bool inhibit = button::inhibitSleep;
      bool graceOver = (now2 - g_bootMs) > BOOT_GRACE_MS;
      bool calibrating = cap::calibrating();
      if (g_isConnected && !g_leak && !inhibit && graceOver && !button::holdActive && !calibrating) {
        pwr::deepSleepSec(DEBUG_SHORT_SLEEP ? DEBUG_SLEEP_OK_SEC : SLEEP_OK_SEC);
      }
    }
  }

  ota::handle();
  delay(5);

  buzzer::tick(BUZZ_ON_MS, BUZZ_PERIOD_MS, BUZZ_FREQ_HZ);

  delay(5);
}
