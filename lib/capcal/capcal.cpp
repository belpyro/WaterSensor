#include "capcal.hpp"
#include <LittleFS.h>
#include <ArduinoJson.h>

// ===== Параметры по умолчанию (при необходимости меняйте до cap::begin) =====
namespace cap {
uint8_t  samples    = 4;
float    emaAlpha   = 0.02f;     // медленная EMA базы "сухо"
float    kFactor    = 1.60f;     // th_on = dry_ema * k + offset
uint32_t offset_us  = 1500;      // мкс
uint32_t hyst_us    = 2000;      // мкс (гистерезис вниз)
uint8_t  confirmOn  = 2;         // 2 подряд превышения → "мокро"
uint8_t  confirmOff = 3;         // 3 подряд ниже th_off → "сухо"
} // namespace cap

namespace {

struct Cfg {
  uint32_t dry_ema = 8000;   // стартовая оценка "сухо" (мкс) под большую C
  float    alpha   = cap::emaAlpha;
  float    k       = cap::kFactor;
  uint32_t offs    = cap::offset_us;
  uint32_t hyst    = cap::hyst_us;
};

const char* kFile = "/capcal.json";

uint8_t  s_pin = 255;
uint16_t s_charge_us = 400;
uint32_t s_timeout_us = 200000;

uint32_t s_lastUs = 0;   // последнее усреднённое измерение
uint32_t s_dryEma = 8000;
bool     s_wet    = false;
uint8_t  s_hitsWet = 0, s_hitsDry = 0;
bool     s_calibrated  = false;   // новый флаг

// --- состояние неблокирующей калибровки ---
bool     s_calibActive   = false;
uint32_t s_calibEndMs    = 0;
uint16_t s_calibStepMs   = 20;
uint64_t s_calibAcc      = 0;
uint32_t s_calibN        = 0;
uint32_t s_calibNextMs   = 0;

// простая «мягкая» задержка в микросекундах с yield() для WiFi
static inline void waitUs(uint32_t us) {
  uint32_t t0 = micros();
  while (micros() - t0 < us) { yield(); }
}

// одно измерение времени до падения в LOW (однопиновый метод)
static inline uint32_t capReadOnce(uint8_t pin, uint16_t charge_us, uint32_t timeout_us) {
  // разряд
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(20);

  // заряд
  digitalWrite(pin, HIGH);
  delayMicroseconds(charge_us);

  // измерение
  pinMode(pin, INPUT); // Hi-Z
  uint32_t t0 = micros();
  while (micros() - t0 < timeout_us) {
    if (digitalRead(pin) == LOW) break;
    yield();
  }
  return micros() - t0;
}

static bool loadCfg(Cfg& c) {
  if (!LittleFS.begin()) LittleFS.begin();
  File f = LittleFS.open(kFile, "r");
  if (!f) return false;
  JsonDocument doc;
  if (deserializeJson(doc, f) != DeserializationError::Ok) { f.close(); return false; }
  c.dry_ema = doc["dry_ema"] | c.dry_ema;
  c.alpha   = doc["alpha"]   | c.alpha;
  c.k       = doc["k"]       | c.k;
  c.offs    = doc["offs"]    | c.offs;
  c.hyst    = doc["hyst"]    | c.hyst;
  f.close();
  return true;
}

static bool saveCfg(const Cfg& c) {
  if (!LittleFS.begin()) LittleFS.begin();
  File f = LittleFS.open(kFile, "w");
  if (!f) return false;
  JsonDocument doc;
  doc["dry_ema"] = c.dry_ema;
  doc["alpha"]   = c.alpha;
  doc["k"]       = c.k;
  doc["offs"]    = c.offs;
  doc["hyst"]    = c.hyst;
  bool ok = (serializeJson(doc, f) > 0);
  f.close();
  return ok;
}

} // namespace

namespace cap {

void begin(uint8_t pin, uint16_t charge_us, uint32_t timeout_us) {
  s_pin = pin;
  s_charge_us = charge_us;
  s_timeout_us = timeout_us;
  pinMode(s_pin, INPUT);

  Cfg c;
  if (loadCfg(c)) {
    emaAlpha  = c.alpha;
    kFactor   = c.k;
    offset_us = c.offs;
    hyst_us   = c.hyst;
    s_dryEma  = c.dry_ema;
    s_calibrated = true;
    Serial.printf("[CAP] loaded: dry=%luus k=%.2f offs=%luus hyst=%luus\n",
                  (unsigned long)s_dryEma, (double)kFactor,
                  (unsigned long)offset_us, (unsigned long)hyst_us);
  } else {
    s_dryEma = 8000; // стартовая оценка "сухо"
    s_calibrated = false;
    Serial.println(F("[CAP] no capcal.json, using defaults"));
  }
}

void tick() {
  if (s_pin == 255) return;

  // усреднение нескольких измерений
  uint64_t acc = 0;
  for (uint8_t i=0; i<samples; ++i) {
    acc += capReadOnce(s_pin, s_charge_us, s_timeout_us);
    delay(2);
  }
  s_lastUs = (uint32_t)(acc / samples);

  // вычисление порогов
  uint32_t th_on  = (uint32_t)(s_dryEma * kFactor) + offset_us;
  uint32_t th_off = (th_on > hyst_us) ? (th_on - hyst_us) : 0;

  // подтверждаем "мокро"/"сухо"
  if (!s_wet) {
    if (s_lastUs > th_on) {
      if (++s_hitsWet >= confirmOn) { s_wet = true; s_hitsWet = 0; s_hitsDry = 0; }
    } else {
      s_hitsWet = 0;
      // только когда "сухо", медленно подстраиваем базу (EMA)
      s_dryEma = (uint32_t)((1.0f - emaAlpha) * s_dryEma + emaAlpha * s_lastUs);
    }
  } else {
    if (s_lastUs < th_off) {
      if (++s_hitsDry >= confirmOff) { s_wet = false; s_hitsWet = 0; s_hitsDry = 0; }
    } else {
      s_hitsDry = 0;
    }
  }

  // шаг неблокирующей калибровки (если активна)
  (void)calibrateStep();

  // разгрузка CPU
  waitUs(2000);
}

bool isWet() { return s_wet && s_calibrated; }

Stats stats() {
  uint32_t th_on  = (uint32_t)(s_dryEma * kFactor) + offset_us;
  uint32_t th_off = (th_on > hyst_us) ? (th_on - hyst_us) : 0;
  bool wetOut = s_calibrated ? s_wet : false;
  return Stats{ s_lastUs, s_dryEma, th_on, th_off, s_calibrated, wetOut, s_hitsWet, s_hitsDry };
}

bool forceSave() {
  Cfg c;
  c.dry_ema = s_dryEma;
  c.alpha   = emaAlpha;
  c.k       = kFactor;
  c.offs    = offset_us;
  c.hyst    = hyst_us;
  bool ok = saveCfg(c);
  Serial.printf("[CAP] forceSave: dry=%luus saved=%d\n",
                (unsigned long)s_dryEma, ok);
  return ok;
}

bool reset() {
  if (!LittleFS.begin()) LittleFS.begin();
  bool ok = true;
  if (LittleFS.exists(kFile)) ok = LittleFS.remove(kFile);
  s_dryEma = 8000;
  s_wet = false;
  s_hitsWet = s_hitsDry = 0;
  s_calibActive = false;
  s_calibAcc = 0; s_calibN = 0;
  s_calibrated = false;
  Serial.println(F("[CAP] reset to defaults, file removed"));
  return ok;
}

// ===== автокалибровка базы "сухо" =====

bool calibrateDryBlocking(uint32_t duration_ms, uint16_t sample_interval_ms) {
  if (s_pin == 255) return false;
  uint32_t tEnd = millis() + duration_ms;
  uint64_t acc = 0;
  uint32_t n = 0;

  while ((int32_t)(millis() - tEnd) < 0) {
    uint64_t one = 0;
    for (uint8_t i = 0; i < samples; ++i) {
      one += capReadOnce(s_pin, s_charge_us, s_timeout_us);
      delay(2);
    }
    acc += (one / samples);
    ++n;

    uint32_t t0 = millis();
    while ((millis() - t0) < sample_interval_ms) { yield(); }
  }

  if (n == 0) return false;

  s_dryEma = (uint32_t)(acc / n);
  Cfg c;
  c.dry_ema = s_dryEma;
  c.alpha   = emaAlpha;
  c.k       = kFactor;
  c.offs    = offset_us;
  c.hyst    = hyst_us;
  bool ok = saveCfg(c);
  s_calibrated = true;
  Serial.printf("[CAP] calibrateDryBlocking: dry=%luus saved=%d (N=%lu)\n",
                (unsigned long)s_dryEma, ok, (unsigned long)n);
  return ok;
}

void startCalibrateDry(uint32_t duration_ms, uint16_t sample_interval_ms) {
  s_calibActive = true;
  s_calibEndMs  = millis() + duration_ms;
  s_calibStepMs = sample_interval_ms;
  s_calibAcc    = 0;
  s_calibN      = 0;
  s_calibNextMs = millis();
  Serial.printf("[CAP] startCalibrateDry: %lu ms, step=%u ms\n",
                (unsigned long)duration_ms, (unsigned)sample_interval_ms);
}

bool calibrateStep() {
  if (!s_calibActive) return false;

  uint32_t now = millis();

  // окончен интервал калибровки
  if ((int32_t)(now - s_calibEndMs) >= 0) {
    s_calibActive = false;
    if (s_calibN == 0) return true;

    s_dryEma = (uint32_t)(s_calibAcc / s_calibN);
    Cfg c;
    c.dry_ema = s_dryEma;
    c.alpha   = emaAlpha;
    c.k       = kFactor;
    c.offs    = offset_us;
    c.hyst    = hyst_us;
    bool ok = saveCfg(c);
    s_calibrated = true;
    Serial.printf("[CAP] calibrateStep: dry=%luus saved=%d (N=%lu)\n",
                  (unsigned long)s_dryEma, ok, (unsigned long)s_calibN);
    return true;
  }

  // время взять новый образец?
  if ((int32_t)(now - s_calibNextMs) >= 0) {
    uint64_t one = 0;
    for (uint8_t i = 0; i < samples; ++i) {
      one += capReadOnce(s_pin, s_charge_us, s_timeout_us);
      delay(2);
    }
    s_calibAcc += (one / samples);
    ++s_calibN;
    s_calibNextMs = now + s_calibStepMs;
  }
  return false; // ещё идёт калибровка
}

bool calibrating() { return s_calibActive; }

} // namespace cap