#include <ESP8266mDNS.h>
#include <constants.hpp>

namespace mdnsu {
  void setup() {
    if (MDNS.begin(MDNS_NAME)) {
      MDNS.addService("http", "tcp", 80);
      Serial.printf("[mDNS] http://%s.local\n", MDNS_NAME);
    } else {
      Serial.println("[mDNS] Failed to start");
    }
  }
}