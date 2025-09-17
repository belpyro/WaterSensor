#include <ESP8266WiFi.h>
#include <constants.hpp>
#if USE_WIFIMANAGER
  #include <WiFiManager.h>
  #include <DNSServer.h>
  #include <ESP8266WebServer.h>
#endif

// опционально: отладка — покажем момент сохранения
static void onSaved() {
  Serial.println(F("[WiFi] SaveConfigCallback: creds/params saved"));
}

namespace wifiu {
  bool setupWiFi() {
  #if USE_WIFIMANAGER
    WiFiManager wm;
    
    wm.setHostname(MDNS_NAME);
    wm.setConfigPortalBlocking(true);
    wm.setBreakAfterConfig(false);     // выйти из autoConnect() сразу после сохранения/коннекта
    wm.setWiFiAutoReconnect(true);
    wm.setCleanConnect(true);
    wm.setSaveConfigCallback(onSaved);
    wm.setConfigPortalTimeout(5*60);
    wm.setConnectTimeout(30);
    wm.setMinimumSignalQuality(20);

    // ВАЖНО: разрешить запись кредов в SDK flash
    // WiFi.persistent(true);

    bool ok = wm.autoConnect("Water_Sensor_AP");
    if (!ok) {
      Serial.println(F("[WiFi] WiFiManager: timeout/fail → portal active. ESP will be rebooted"));
      ESP.restart();
    }
    Serial.printf("[WiFi] Connected: %s (%s)\n",
                  WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    return true;
  #else
    const char* SSID = "YourSSID";
    const char* PASS = "YourPASS";
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin(SSID, PASS);
    Serial.print(F("[WiFi] Connecting"));
    while (WiFi.status() != WL_CONNECTED) { Serial.print('.'); delay(500); }
    Serial.printf("\n[WiFi] OK: %s\n", WiFi.localIP().toString().c_str());
    return true;
  #endif
  }
}