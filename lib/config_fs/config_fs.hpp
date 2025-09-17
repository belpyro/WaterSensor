#pragma once
#include <types.hpp>

namespace cfgfs {
  extern const char* CONFIG_PATH;
  bool beginFS();
  void load(AppConfig& cfg);
  void save(const AppConfig& cfg);
}