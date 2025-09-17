#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <constants.hpp>

namespace ota {

  void begin() {
#if OTA_ENABLE
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setHostname(MDNS_NAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([](){
      Serial.println(F("[OTA] Start"));
    });
    ArduinoOTA.onEnd([](){
      Serial.println(F("\n[OTA] End"));
    });
    ArduinoOTA.onProgress([](unsigned int prog, unsigned int total){
      static unsigned int last = 0;
      unsigned int pct = (prog * 100U) / total;
      if (pct != last) { last = pct; Serial.printf("\r[OTA] %u%%", pct); }
    });
    ArduinoOTA.onError([](ota_error_t e){
      Serial.printf("\n[OTA] Error %u\n", (unsigned)e);
    });

    ArduinoOTA.begin();
    Serial.printf("[OTA] Ready on %s:%u (host=%s)\n",
                  WiFi.localIP().toString().c_str(), OTA_PORT, MDNS_NAME);
#endif
  }

  void handle() {
#if OTA_ENABLE
    ArduinoOTA.handle();
#endif
  }

} // namespace ota