#include "cap_api.hpp"
#include "capcal.hpp"
#include <ArduinoJson.h>

using namespace cap;

namespace {

  template<typename Srv>
  void sendJson(Srv &srv, const JsonDocument &doc, int code = 200) {
    String out; serializeJson(doc, out);
    srv.send(code, "application/json", out);
  }

  template<typename Srv>
  void handleStatus(Srv &srv){
    auto s = cap::stats();
    JsonDocument doc;
    doc["t_us"]        = s.t_us;
    doc["dry_ema"]     = s.dry_ema;
    doc["th_on"]       = s.th_on;
    doc["th_off"]      = s.th_off;
    doc["calibrated"]  = s.calibrated;
    doc["wet"]         = s.wet;
    doc["hitsWet"]     = s.hitsWet;
    doc["hitsDry"]     = s.hitsDry;
    doc["calibrating"] = cap::calibrating();
    sendJson(srv, doc);
  }

  template<typename Srv>
  void handleStart(Srv &srv){
    uint32_t ms   = srv.hasArg("ms")   ? srv.arg("ms").toInt()   : 5000;
    uint16_t step = srv.hasArg("step") ? srv.arg("step").toInt() : 20;
    cap::startCalibrateDry(ms, step);
    srv.send(202, "text/plain", "calibration started");
  }

  template<typename Srv>
  void handleBlocking(Srv &srv){
    uint32_t ms   = srv.hasArg("ms")   ? srv.arg("ms").toInt()   : 5000;
    uint16_t step = srv.hasArg("step") ? srv.arg("step").toInt() : 20;
    bool ok = cap::calibrateDryBlocking(ms, step);
    srv.send(ok ? 200 : 500, "text/plain", ok ? "ok" : "fail");
  }

  template<typename Srv>
  void handleReset(Srv &srv){
    bool ok = cap::reset();
    srv.send(ok ? 200 : 500, "text/plain", ok ? "reset" : "reset-failed");
  }

  template<typename Srv>
  void handleSave(Srv &srv){
    bool ok = cap::forceSave();
    srv.send(ok ? 200 : 500, "text/plain", ok ? "saved" : "save-failed");
  }

  template<typename Srv>
  void handleGetCfg(Srv &srv){
    JsonDocument doc;
    doc["samples"]    = cap::samples;
    doc["alpha"]      = cap::emaAlpha;
    doc["k"]          = cap::kFactor;
    doc["offset_us"]  = cap::offset_us;
    doc["hyst_us"]    = cap::hyst_us;
    doc["confirmOn"]  = cap::confirmOn;
    doc["confirmOff"] = cap::confirmOff;
    sendJson(srv, doc);
  }

  template<typename Srv>
  void handleSetCfg(Srv &srv){
    if (!srv.hasArg("plain")) { srv.send(400, "text/plain", "no body"); return; }
    JsonDocument doc;
    DeserializationError e = deserializeJson(doc, srv.arg("plain"));
    if (e) { srv.send(400, "text/plain", "bad json"); return; }

    if (doc.containsKey("samples"))    cap::samples    = (uint8_t)doc["samples"].as<int>();
    if (doc.containsKey("alpha"))      cap::emaAlpha   = doc["alpha"].as<float>();
    if (doc.containsKey("k"))          cap::kFactor    = doc["k"].as<float>();
    if (doc.containsKey("offset_us"))  cap::offset_us  = (uint32_t)doc["offset_us"].as<unsigned long>();
    if (doc.containsKey("hyst_us"))    cap::hyst_us    = (uint32_t)doc["hyst_us"].as<unsigned long>();
    if (doc.containsKey("confirmOn"))  cap::confirmOn  = (uint8_t)doc["confirmOn"].as<int>();
    if (doc.containsKey("confirmOff")) cap::confirmOff = (uint8_t)doc["confirmOff"].as<int>();

    srv.send(200, "text/plain", "ok");
  }

} // namespace

namespace capapi {
#ifdef ESP8266
void attach(ESP8266WebServer &srv) {
#else
void attach(WebServer &srv) {
#endif
  srv.on("/cap/status",              HTTP_GET,  [&](){ handleStatus(srv); });
  srv.on("/cap/calibrate/start",     HTTP_POST, [&](){ handleStart(srv); });
  srv.on("/cap/calibrate/blocking",  HTTP_POST, [&](){ handleBlocking(srv); });
  srv.on("/cap/calibrate/reset",     HTTP_POST, [&](){ handleReset(srv); });
  srv.on("/cap/save",                HTTP_POST, [&](){ handleSave(srv); });
  srv.on("/cap/config",              HTTP_GET,  [&](){ handleGetCfg(srv); });
  srv.on("/cap/config",              HTTP_POST, [&](){ handleSetCfg(srv); });
}
} // namespace capapi