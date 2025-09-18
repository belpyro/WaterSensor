#ifndef ESPWATERTEST_CAP_CALIBRATOR_HPP
#define ESPWATERTEST_CAP_CALIBRATOR_HPP
#include <Arduino.h>

namespace cap {

struct Stats {
  uint32_t t_us;       // последнее усреднённое измерение (мкс)
  uint32_t dry_ema;    // скользящая база "сухо" (мкс)
  uint32_t th_on;      // порог включения "мокро" (мкс)
  uint32_t th_off;     // порог выключения (мкс, с гистерезисом)
  bool     calibrated; // пройдена ли калибровка (есть валидная база)
  bool     wet;        // текущее состояние
  uint8_t  hitsWet;    // счётчик подтверждений "мокро"
  uint8_t  hitsDry;    // счётчик подтверждений "сухо"
};

// Инициализация. pin — GPIO, подключённый к площадке (через ~1k к площадке и Rpd на GND).
// charge_us — время зарядки площадки HIGH перед измерением; timeout_us — максимум ожидания разряда.
void begin(uint8_t pin, uint16_t charge_us = 400, uint32_t timeout_us = 200000);

// Вызывать часто в loop (можно 5–10 раз/сек). Внутри делает одно измерение и обновляет состояние.
void tick();

bool isWet();
Stats stats();

// --- настройки (можно изменить из приложения до begin()) ---
extern uint8_t  samples;     // количество усреднений за одно измерение (по умолчанию 4)
extern float    emaAlpha;    // коэффициент EMA базы "сухо" (0..1), напр. 0.02
extern float    kFactor;     // множитель порога: th_on = dry_ema*k + offset
extern uint32_t offset_us;   // добавка к порогу (мкс)
extern uint32_t hyst_us;     // гистерезис (мкс)
extern uint8_t  confirmOn;   // N подряд выше th_on для включения "мокро"
extern uint8_t  confirmOff;  // N подряд ниже th_off для выключения

// --- управление калибровкой и сохранением ---
bool forceSave();            // принудительно сохранить текущую калибровку в /capcal.json
bool reset();                // удалить /capcal.json и сбросить к дефолту (сухо)

// --- автокалибровка (dry) ---
// Блокирующая: собирает выборку duration_ms, сохраняет базу "сухо".
bool calibrateDryBlocking(uint32_t duration_ms = 5000, uint16_t sample_interval_ms = 20);

// Неблокирующая: запустить, затем вызывать calibrateStep() в loop().
// calibrateStep() вернёт true по завершении (и выполнит сохранение).
void startCalibrateDry(uint32_t duration_ms = 5000, uint16_t sample_interval_ms = 20);
bool calibrateStep();
bool calibrating();  // идёт ли сейчас калибровка

} // namespace cap
#endif