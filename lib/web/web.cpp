#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <web.hpp>
#include <config_fs.hpp>
#include <index_fallback.hpp>
#include <constants.hpp>
#include <cap_api.hpp>

static ESP8266WebServer server(80);
static AppConfig current;
static ESP8266HTTPUpdateServer httpUpdater;

static void setupWebOta() {
#if WEB_OTA_ENABLE
  // защищаем загрузчик BasicAuth'ом теми же логином/паролем
  httpUpdater.setup(&server, "/update", BASIC_AUTH_USER, BASIC_AUTH_PASS);
  Serial.println("[HTTP] Web OTA at /update");
#endif
}

static bool ensureAuth() {
  if (!server.authenticate(BASIC_AUTH_USER, BASIC_AUTH_PASS)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

static void handleRoot() {
  if (LittleFS.exists("/index.html")) {
    File f = LittleFS.open("/index.html", "r");
    server.streamFile(f, "text/html");
    f.close();
    return;
  }
  server.send_P(200, "text/html", INDEX_FALLBACK);
}

static void handleGetConfig() {
  JsonDocument doc;
  doc["deviceName"]   = current.deviceName;
  doc["mqttServer"]   = current.mqttServer;
  doc["mqttPort"]     = current.mqttPort;
  doc["alarmEnabled"] = current.alarmEnabled;
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

static void handlePostConfig() {
  if (!ensureAuth()) return;
  if (!server.hasArg("plain")) { server.send(400, "text/plain", "no body"); return; }

  JsonDocument doc;
  auto err = deserializeJson(doc, server.arg("plain"));
  if (err) { server.send(400, "text/plain", String("bad json: ")+err.c_str()); return; }

  const char* dn = doc["deviceName"] | current.deviceName;
  const char* ms = doc["mqttServer"] | current.mqttServer;
  uint16_t mp   = doc["mqttPort"] | current.mqttPort;
  bool ae       = doc["alarmEnabled"] | current.alarmEnabled;

  strlcpy(current.deviceName, dn, sizeof(current.deviceName));
  strlcpy(current.mqttServer, ms, sizeof(current.mqttServer));
  current.mqttPort = mp;
  current.alarmEnabled = ae;

  cfgfs::save(current);
  server.send(200, "text/plain", "saved");
}

static void handleReboot() {
  if (!ensureAuth()) return;
  server.send(200, "text/plain", "rebooting");
  delay(250);
  ESP.restart();
}

namespace web {
  void begin(const AppConfig* cfg) {
    current = *cfg;

    server.on("/", HTTP_GET, handleRoot);
    server.on("/config", HTTP_GET, handleGetConfig);
    server.on("/config", HTTP_POST, handlePostConfig);
    server.on("/reboot", HTTP_POST, handleReboot);
    server.onNotFound([](){ server.send(404, "text/plain", "Not Found"); });
    capapi::attach(server);
    server.begin();
    setupWebOta();
    Serial.println("[HTTP] Server started");
  }

  void loop() {
    server.handleClient();
  }
}