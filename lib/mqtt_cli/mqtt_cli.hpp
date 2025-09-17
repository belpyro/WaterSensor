#ifndef ESPWATERTEST_MQTT_HPP
#define ESPWATERTEST_MQTT_HPP
#include <types.hpp>

namespace mqt {
  void configure(const AppConfig& cfg);
  bool publishStatus(bool leak);
  bool publishActive();
}
#endif