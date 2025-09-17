#include <ESP8266WiFi.h>
#include <power.hpp>

namespace pwr {
  void deepSleepSec(uint32_t seconds) {
    Serial.printf("[PM] Deep sleep for %lu sec...\n", (unsigned long)seconds);
    // WiFi.disconnect(false);
    // WiFi.mode(WIFI_OFF);
    delay(50);
    ESP.deepSleep((uint64_t)seconds * 1000000ULL, WAKE_RF_DEFAULT);
  }
}