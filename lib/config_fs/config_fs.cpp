#include <LittleFS.h>
#include <ArduinoJson.h>
#include <config_fs.hpp>

namespace cfgfs {
  const char* CONFIG_PATH = "/config.json";

  static void writeDefaults(AppConfig& cfg) {
    strlcpy(cfg.deviceName, "esp8266", sizeof(cfg.deviceName));
    strlcpy(cfg.mqttServer, "mqtt.local", sizeof(cfg.mqttServer));
    cfg.mqttPort = 1883;
    cfg.alarmEnabled = true;
  }

  bool beginFS() {
    if (!LittleFS.begin()) {
      Serial.println("[FS] LittleFS mount failed");
      return false;
    }
    Serial.println("[FS] LittleFS mounted");
    return true;
  }

  void load(AppConfig& cfg) {
    if (!LittleFS.exists(CONFIG_PATH)) {
      Serial.println("[FS] No config.json, writing defaults");
      writeDefaults(cfg);
      save(cfg);
      return;
    }
    File f = LittleFS.open(CONFIG_PATH, "r");
    if (!f) { Serial.println("[FS] open read failed"); writeDefaults(cfg); return; }
    DynamicJsonDocument doc(512);
    auto err = deserializeJson(doc, f);
    f.close();
    if (err) {
      Serial.printf("[FS] JSON parse error: %s\n", err.c_str());
      writeDefaults(cfg);
      return;
    }
    strlcpy(cfg.deviceName, doc["deviceName"] | "esp8266", sizeof(cfg.deviceName));
    strlcpy(cfg.mqttServer, doc["mqttServer"] | "mqtt.local", sizeof(cfg.mqttServer));
    cfg.mqttPort = doc["mqttPort"] | 1883;
    cfg.alarmEnabled = doc["alarmEnabled"] | true;
    Serial.println("[FS] Config loaded");
  }

  void save(const AppConfig& cfg) {
    File f = LittleFS.open(CONFIG_PATH, "w");
    if (!f) { Serial.println("[FS] open write failed"); return; }
    DynamicJsonDocument doc(512);
    doc["deviceName"]   = cfg.deviceName;
    doc["mqttServer"]   = cfg.mqttServer;
    doc["mqttPort"]     = cfg.mqttPort;
    doc["alarmEnabled"] = cfg.alarmEnabled;
    serializeJson(doc, f);
    f.close();
    Serial.println("[FS] Config saved");
  }
}