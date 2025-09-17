#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <mqtt_cli.hpp>
#include <constants.hpp>

static WiFiClient s_net;
static PubSubClient s_cli(s_net);
static String s_host;
static uint16_t s_port;

namespace mqt {
  void configure(const AppConfig& cfg) {
    s_host = (cfg.mqttServer && cfg.mqttServer[0]) ? String(cfg.mqttServer) : String(DEFAULT_MQTT_HOST);
    s_port = cfg.mqttPort ? cfg.mqttPort : DEFAULT_MQTT_PORT;
    s_cli.setServer(s_host.c_str(), s_port);
  }

  bool publishStatus(bool leak) {
    if (!s_cli.connected()) {
      if (!s_cli.connect(MQTT_CLIENT_ID)) {
        Serial.printf("[MQTT] connect failed, rc=%d\n", s_cli.state());
        return false;
      }
    }
    StaticJsonDocument<160> doc;
    doc["device"] = HOSTNAME;
    doc["ssid"]   = WiFi.SSID();
    doc["ip"]     = WiFi.localIP().toString();
    doc["rssi"]   = WiFi.RSSI();
    doc["leak"]   = leak;
    char payload[160];
    size_t n = serializeJson(doc, payload, sizeof(payload));
    (void)n;
    bool ok = s_cli.publish(MQTT_TOPIC_STATUS, payload, true);
    Serial.printf("[MQTT] publish %s: %s\n", ok ? "OK" : "FAIL", payload);
    s_cli.disconnect();
    return ok;
  }
}