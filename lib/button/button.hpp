#ifndef ESPWATERTEST_BUTTON_TEST_HPP
#define ESPWATERTEST_BUTTON_TEST_HPP
#include <Arduino.h>

namespace button {

  // инициализация пина (active low с INPUT_PULLUP)
  void begin(uint8_t pin);

  // вызывать в loop() — обработка нажатий
  void tick();

  extern bool testLeak;       // короткое нажатие → тест протечки
  extern bool inhibitSleep;   // длинное нажатие → запрет сна
  extern bool resetConfig;    // очень длинное → сброс конфига
  extern bool holdActive;   // true while button is currently pressed

  using ResetCb = void(*)(bool fullWifi);
  void setResetCallback(ResetCb cb); // назначить обработчик сброса
  void requestReset(bool fullWifi);  // программно запросить сброс (извне)
}
#endif