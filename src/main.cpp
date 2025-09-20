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

  cfgfs::beginFS();
  cfgfs::load(g_cfg);

  g_isConnected = wifiu::setupWiFi();
  mdnsu::setup();
  ota::begin();
  web::begin(&g_cfg);
  sensor::begin(PIN_VREF);
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
  delay(50);

  bool sensorLeak = sensor::readLeak(DEBUG_FORCE_LEAK);

  if(sensorLeak != g_leak){
    g_leak = sensorLeak;
    buzzer::setLeak(g_leak);
    (void)mqt::publishStatus(g_leak);
  }  

  ota::handle();
  delay(50);

  buzzer::tick(BUZZ_ON_MS, BUZZ_PERIOD_MS, BUZZ_FREQ_HZ);
}
