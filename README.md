# 🚨 Project Sentinel: R-SEC System Smart Alarm

![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif)
![Language](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B)
![IoT](https://img.shields.io/badge/IoT-Blynk-1DB954?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Project Sentinel (R-SEC System)** is a robust, ESP32-based laser tripwire security system. It features real-time perimeter monitoring, hardware-level alerts, and a fully integrated remote dashboard via the **Blynk IoT** platform.

---

## 📑 Table of Contents
- [Key Features](#key-features)
- [Hardware Requirements](#hardware-requirements)
- [Pin Mapping](#pin-mapping)
- [Blynk Configuration](#blynk-configuration)
- [Installation & Setup](#installation--setup)
- [License & Disclaimer](#license--disclaimer)

---

## ✨ Key Features
* **Real-Time Intrusion Detection:** Utilizes a laser beam and LDR sensor to detect unauthorized entry instantly.
* **Smart Anti-Spam Logic:** Prevents notification flooding by locking the alert state until the perimeter is secured.
* **Hybrid Connectivity:** Automatically switches between Online (Blynk) and Offline modes depending on WiFi availability.
* **Live Dashboard Monitoring:** View real-time security status, raw sensor data charts, and virtual LED indicators directly from a smartphone.
* **Audio-Visual Feedback:** Equipped with an I2C LCD for physical status readouts and a buzzer that plays a custom warning melody.

---

## 🛠️ Hardware Requirements
| Component | Quantity | Notes |
| :--- | :---: | :--- |
| **ESP32** Development Board | 1 | Core microcontroller |
| **LDR Sensor** (Photoresistor) | 1 | Laser beam receiver |
| **Laser Pointer** Module | 1 | 5V or 3.3V compatible |
| **16x2 LCD Display** | 1 | Must include I2C Backpack |
| **Active Buzzer** | 1 | Audio alarm output |
| **LED** (Red) | 1 | Visual alarm indicator |
| **Breadboard & Jumper Wires** | 1 set | For circuit prototyping |

---

## 🔌 Pin Mapping (Wiring)
Ensure your hardware is connected according to this schematic configuration:

| Component | ESP32 Pin | Type | Notes |
| :--- | :---: | :---: | :--- |
| **LDR Sensor** | `GPIO 32` | `Analog In` | Reads laser intensity |
| **Buzzer** | `GPIO 4` | `Digital Out` | Audio alarm |
| **LED Alert** | `GPIO 27` | `Digital Out` | Visual alarm |
| **LCD SDA** | `GPIO 21` | `I2C` | Default I2C SDA pin |
| **LCD SCL** | `GPIO 22` | `I2C` | Default I2C SCL pin |

---

## ☁️ Blynk Datastream Configuration
Set up the following Datastreams in your Blynk Web Console:

| Virtual Pin | Name | Data Type | Min / Max | Widget Used in App |
| :---: | :--- | :---: | :---: | :--- |
| `V1` | Security Status | `String` | - | Value Display |
| `V2` | Sensor Graphic | `Integer` | 0 - 4095 | Gauge & Line Chart |
| `V3` | LED Indicator | `Integer` | 0 - 1 | LED Widget |

> **⚠️ Important Event Setup:**
> Create a new Event in the Blynk Console with the Event Code `peringatan_bahaya`. Enable the **"Deliver push notifications as alerts"** option to ensure the smartphone alarm rings loudly.

---

## 🚀 Installation & Setup

### 1. Clone the Repository

Download the project files to your local machine:

```bash
git clone https://github.com/Jejekdf/r-sec-smart-alarm.git
cd r-sec-smart-alarm
```

### 2. Install Required Libraries

Ensure you have the following libraries installed in your Arduino IDE or PlatformIO environment:

- `WiFi.h` (Built-in for ESP32)
- `BlynkSimpleEsp32.h`
- `LiquidCrystal_I2C.h`

### 3. Configure Credentials

Open the main source file (`main.cpp` or `.ino`) and locate the configuration section. Replace the placeholders with your actual credentials:

```cpp
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Project Sentinel"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
```

### 4. Compile and Upload

1. Connect the ESP32 to your computer
2. Select the correct **COM Port** and **Board Model** (e.g., *ESP32 Dev Module*)
3. Upload the code to the board

### 5. Calibration

Monitor the LCD screen during normal operation. If the ambient light or laser intensity varies, adjust the `safeThreshold` variable in the code to ensure accurate triggering.

---

## 🛡️ License & Disclaimer
This project is developed for **educational purposes**. Always ensure hardware connections are secure to prevent short circuits. 
Distributed under the MIT License. See `LICENSE` for more information.