

# EspWaterTest

IoT water leak sensor based on **ESP8266**, publishing state via MQTT and integrating with Homebridge / Apple HomeKit.

## ✨ Features
- Leak detection using an analog sensor.
- Audible alarm with piezo buzzer.
- Web configuration interface (WiFiManager + LittleFS).
- Automatic configuration persistence (`config.json`).
- mDNS (`http://esp8266.local`).
- MQTT integration (`home/water/status`) → easy to connect with Homebridge.
- **Deep sleep** support for battery operation.
- OTA updates (planned).

## 📂 Project Structure
```
EspWaterTest/
├─ src/
│  └─ main.cpp              # entry point
├─ lib/
│  ├─ common/               # constants and types
│  ├─ config_fs/            # LittleFS configuration handling
│  ├─ wifi/                 # WiFiManager and auto-connect
│  ├─ mdns/                 # mDNS service
│  ├─ web/                  # web server and index fallback
│  ├─ sensor/               # leak sensor
│  ├─ buzzer/               # buzzer control
│  ├─ mqtt_cli/             # MQTT client
│  └─ power/                # deep sleep management
├─ data/
│  ├─ index.html            # configuration page (uploaded to LittleFS)
│  └─ config.json           # default configuration
├─ docker-compose.yaml      # Mosquitto + Homebridge
└─ platformio.ini           # PlatformIO settings
```

## ⚙️ Firmware Setup
1. Install [PlatformIO](https://platformio.org/).
2. Clone the repository and build:
   ```bash
   pio run
   ```
3. Upload firmware:
   ```bash
   pio run -t upload
   ```
4. Upload filesystem (HTML + config):
   ```bash
   pio run -t buildfs
   pio run -t uploadfs
   ```

## 🔌 MQTT Integration
ESP publishes JSON into topic `home/water/status`:

```json
{
  "device": "WaterSensor",
  "ssid": "TEST",
  "ip": "192.168.1.175",
  "rssi": -60,
  "leak": false
}
```

- `leak = true/false` — leak state.  
- Topic can be redefined in `constants.hpp`.

### Example for Homebridge (mqttthing plugin)
```json
{
  "accessory": "mqttthing",
  "type": "leakSensor",
  "name": "Water Leak",
  "url": "mqtt://mosquitto:1883",
  "username": "homeuser",
  "password": "YOUR_PASSWORD",
  "topics": {
    "getLeakDetected": {
      "topic": "home/water/status",
      "apply": "return JSON.parse(message).leak ? 1 : 0;"
    }
  }
}
```

## 🐳 Docker (MQTT + Homebridge)
`docker-compose.yaml` is included:

- `mosquitto` — MQTT broker.  
- `homebridge` — Homebridge with web UI (port `8581`).

Start:
```bash
docker compose up -d
```

## 🔋 Power
- Supports **1S or 2S Li-ion** supply.  
- In deep sleep mode:  
  - 5 s active per hour → up to 2 years (with good regulator),  
  - 30 s active per hour → ~5 months.  
- Critical: use a regulator with very low quiescent current (µA range).

## 📌 Roadmap
- OTA firmware updates.  
- Extended MQTT (siren ON/OFF).  
- Graphs and logs in the web UI.  

---

👨‍💻 Author: **belpyro**  
License: MIT