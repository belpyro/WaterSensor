#ifndef ESPWATERTEST_SENSOR_HPP
#define ESPWATERTEST_SENSOR_HPP
#include <Arduino.h>

namespace sensor {
  void begin(uint8_t pinVref);
  bool readLeak(int adcThreshold, bool debugForce);
}
#endif