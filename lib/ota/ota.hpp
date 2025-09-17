#ifndef ESPWATERTEST_OTA_HPP
#define ESPWATERTEST_OTA_HPP
namespace ota {
  void begin();   // init ArduinoOTA
  void handle();  // call in loop()
}
#endif