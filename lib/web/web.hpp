#ifndef ESPWATERTEST_WEB_HPP
#define ESPWATERTEST_WEB_HPP
#include <types.hpp>
#include <ESP8266HTTPUpdateServer.h>

namespace web {
  void begin(const AppConfig* cfg);
  void loop();
}
#endif