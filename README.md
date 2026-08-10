# ESP32 IoT Environmental Monitor

> End-to-end IoT monitoring system: custom I2C driver for BME280 sensor → ESP-IDF firmware → MQTT protocol → real-time Node-RED dashboard. Built entirely on ESP-IDF without Arduino abstractions.

---

## System Architecture

```
┌─────────────────────────────────────────────────────┐
│                   ESP32 (ESP-IDF)                   │
│                                                     │
│  ┌──────────────┐     ┌────────────────────────┐   │
│  │ BME280 Driver│────▶│   Application Logic    │   │
│  │  (I2C / C)   │     │  temp / humidity / pres│   │
│  └──────────────┘     └──────────┬─────────────┘   │
│                                  │                  │
│                        ┌─────────▼──────────┐      │
│                        │   MQTT Client      │      │
│                        │  (ESP-IDF mqtt)    │      │
│                        └─────────┬──────────┘      │
└──────────────────────────────────┼─────────────────┘
                                   │ TCP/IP (WiFi)
                         ┌─────────▼──────────┐
                         │  Mosquitto Broker  │
                         │     (Local PC)     │
                         └─────────┬──────────┘
                                   │
                         ┌─────────▼──────────┐
                         │  Node-RED Dashboard│
                         │  (Gauges, Charts,  │
                         │   GPIO Control)    │
                         └────────────────────┘
```

---

## Hardware Required

| Component | Purpose |
|-----------|---------|
| ESP32 DevKit | Main microcontroller |
| BME280 module | Temperature, humidity, pressure sensor |
| Jumper wires | I2C connections |
| USB cable | Power and flashing |

### Wiring (I2C)

| BME280 Pin | ESP32 Pin |
|-----------|-----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

---

## Software Dependencies

| Tool | Version |
|------|---------|
| ESP-IDF | v5.x |
| Mosquitto MQTT Broker | 2.x |
| Node-RED | 3.x |
| Python | 3.x (for setup scripts) |

---

## Project Structure

```
esp32-bme280-iot-monitor/
├── main/
│   ├── CMakeLists.txt
│   ├── main.c                  # Application entry point
│   ├── bme280_driver.c         # Custom I2C driver
│   ├── bme280_driver.h
│   ├── mqtt_handler.c          # MQTT publish logic
│   └── mqtt_handler.h
├── docs/
│   └── nodered_flow.json       # Importable Node-RED flow
├── scripts/
│   └── setup_broker.sh         # Mosquitto setup helper
├── config.example.h            # ← Copy to config.h, fill in your values
├── CMakeLists.txt
├── sdkconfig.defaults
├── .gitignore
└── README.md
```

---

## Getting Started

### 1. Clone and configure

```bash
git clone https://github.com/RenoTJ42/esp32-bme280-iot-monitor
cd esp32-bme280-iot-monitor
cp config.example.h main/config.h
# Edit config.h — add your WiFi SSID, password, and broker IP
```

### 2. Build and flash

```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 3. Start Mosquitto broker

```bash
mosquitto -c mosquitto.conf -v
```

### 4. Import Node-RED flow

Open Node-RED → Menu → Import → paste contents of `docs/nodered_flow.json`

---

## Key Implementation Details

### Custom I2C Driver
Rather than using a pre-built BME280 library, the driver is implemented directly using ESP-IDF's `i2c_master` component. This includes:
- I2C bus initialization and device registration
- Reading calibration coefficients from BME280 NVM
- Applying Bosch's compensation formulas for accurate readings

### MQTT Publishing
Sensor data is serialised as a JSON string and published to three topics:
- `sensor/temperature` — in °C
- `sensor/humidity` — in %RH  
- `sensor/pressure` — in hPa

### Node-RED Dashboard
The dashboard provides:
- Live gauge displays for all three parameters
- Historical line charts (last 60 readings)
- Toggle button for remote GPIO control of an ESP32 output pin

---

## What I Learned

Implementing the BME280 driver from scratch — rather than using a library — forced me to work directly with the sensor's datasheet, handle I2C register maps, and apply the manufacturer's multi-step compensation formulas in C. The difference between raw ADC output and a calibrated reading made the value of proper driver implementation very concrete.

