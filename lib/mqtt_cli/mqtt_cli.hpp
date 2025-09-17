#pragma once
#include <types.hpp>

namespace mqt {
  void configure(const AppConfig& cfg);
  bool publishStatus(bool leak);
}