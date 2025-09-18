#pragma once
#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WebServer.h>
#else
  #include <WebServer.h>
#endif

namespace capapi {
  // Регистрирует REST-эндпоинты для калибровки/статуса емкостного датчика.
  // Эндпоинты:
  // GET  /cap/status
  // POST /cap/calibrate/start?ms=5000&step=20
  // POST /cap/calibrate/blocking?ms=5000&step=20
  // POST /cap/calibrate/reset
  // POST /cap/save
  // GET  /cap/config
  // POST /cap/config     (JSON body: {samples,alpha,k,offset_us,hyst_us,confirmOn,confirmOff})
#ifdef ESP8266
  void attach(ESP8266WebServer &srv);
#else
  void attach(WebServer &srv);
#endif
} // namespace capapi