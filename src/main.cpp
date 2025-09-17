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

#define WM_DEBUG_LEVEL

static AppConfig g_cfg;
static bool g_leak = false;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("Booting v%s\n", APP_VERSION);

  pinMode(PIN_VREF, OUTPUT);
  digitalWrite(PIN_VREF, HIGH);
  pinMode(PIN_LEAK_TEST, INPUT_PULLUP);

  cfgfs::beginFS();
  cfgfs::load(g_cfg);

  bool isConnected = wifiu::setupWiFi();
  mdnsu::setup();
  web::begin(&g_cfg);

  sensor::begin(PIN_VREF, PIN_LEAK_TEST);
  buzzer::begin(PIN_BUZZER);

  // Первичное чтение и звуковая индикация
  g_leak = sensor::readLeak(ADC_LEAK_THRESHOLD, DEBUG_FORCE_LEAK);
  buzzer::setLeak(g_leak);
  if (g_leak) tone(PIN_BUZZER, BUZZ_FREQ_HZ, 200);

  // Публикация статуса в MQTT один раз при старте
  mqt::configure(g_cfg);
  (void)mqt::publishStatus(g_leak);

  Serial.printf("Open http://%s.local/ or http://%s/\n", MDNS_NAME, WiFi.localIP().toString().c_str());

  // Режим на батарее: если всё хорошо — уходим спать

  if (BATTERY_MODE) {
    if (isConnected && !g_leak) {
      pwr::deepSleepSec(DEBUG_SHORT_SLEEP ? DEBUG_SLEEP_OK_SEC : SLEEP_OK_SEC);
    } else {
      Serial.println(F("[PM] Stay awake (portal active or leak)"));
    }
  }
}

void loop() {
  web::loop();
  buzzer::tick(BUZZ_ON_MS, BUZZ_PERIOD_MS, BUZZ_FREQ_HZ);

  // Периодическое перечитывание сенсора и публикация изменений
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last > 3000) {
    last = now;
    bool leak = sensor::readLeak(ADC_LEAK_THRESHOLD, DEBUG_FORCE_LEAK);
    if (leak != g_leak) {
      g_leak = leak;
      buzzer::setLeak(g_leak);
      (void)mqt::publishStatus(g_leak);
    }
  }
  delay(5);
}
