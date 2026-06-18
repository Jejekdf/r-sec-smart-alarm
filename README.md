<div align="center">

# 🚨 R-SEC System: IoT Perimeter Smart Alarm
**Project Sentinel**

![ESP32](https://img.shields.io/badge/Board-ESP32-blue?style=for-the-badge&logo=espressif)
![C++](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B)
![Blynk](https://img.shields.io/badge/Platform-Blynk_IoT-1DB954?style=for-the-badge)

*A robust, ESP32-based laser tripwire security system featuring real-time monitoring and cloud integration.*

</div>

---

**Project Sentinel (R-SEC System)** is an advanced hardware security solution. It provides real-time perimeter monitoring, physical hardware alerts (Buzzer & LCD), and a fully integrated remote dashboard via the **Blynk IoT** platform for instant push notifications and live data visualization.

## 📖 Table of Contents
- [✨ Key Features](#-key-features)
- [🛠️ Hardware Requirements](#️-hardware-requirements)
- [🔌 Pin Mapping (Wiring)](#-pin-mapping-wiring)
- [☁️ Blynk Setup](#️-blynk-datastream-configuration)
- [🚀 Installation & Setup](#-installation--setup)

---

## ✨ Key Features
* **🎯 Real-Time Intrusion Detection:** Utilizes a high-precision laser beam and LDR sensor to detect unauthorized entry instantly.
* **🧠 Smart Anti-Spam Logic:** Prevents notification flooding by locking the alert state until the perimeter is properly secured.
* **🌐 Hybrid Connectivity:** Automatically switches between Online (Blynk Connected) and Offline mode depending on your WiFi network's availability.
* **📱 Live Dashboard Monitoring:** View real-time security status, raw sensor data charts, and virtual LED indicators directly from your smartphone.
* **🔊 Audio-Visual Feedback:** Equipped with an I2C LCD for physical status readouts and a buzzer that plays a custom warning melody when triggered.

---

## 🛠️ Hardware Requirements
To replicate this project, you will need the following components:

- [x] **1x** ESP32 Development Board
- [x] **1x** LDR Sensor (Photoresistor)
- [x] **1x** Laser Pointer Module (5V/3.3V)
- [x] **1x** 16x2 LCD Display with I2C Backpack
- [x] **1x** Active Buzzer
- [x] **1x** LED (Red)
- [x] Breadboard & Jumper Wires

---

## 🔌 Pin Mapping (Wiring)
Ensure your hardware is connected securely according to this schematic configuration:

| Component       | ESP32 Pin  | Notes                                   |
| :---            | :---       | :---                                    |
| **LDR Sensor**  | `GPIO 32`  | Analog input for reading laser intensity|
| **Buzzer**      | `GPIO 4`   | Digital output for audio alarm          |
| **LED Alert**   | `GPIO 27`  | Digital output for visual alarm         |
| **LCD SDA**     | `GPIO 21`  | Default I2C SDA pin                     |
| **LCD SCL**     | `GPIO 22`  | Default I2C SCL pin                     |

---

## ☁️ Blynk Datastream Configuration
Set up the following Datastreams in your **Blynk Web Console**:

| Pin  | Name             | Data Type | Min/Max  | Widget Used in App      |
| :--- | :---             | :---      | :---     | :---                    |
| `V1` | Security Status  | `String`  | -        | Value Display           |
| `V2` | Sensor Graphic   | `Integer` | 0 - 4095 | Gauge & Line Chart      |
| `V3` | LED Indicator    | `Integer` | 0 - 1    | LED                     |

> **⚠️ Important Event Setup:**  
> Create a new Event in the Blynk Console with the Event Code `peringatan_bahaya`. **Enable** the *"Deliver push notifications as alerts"* option to ensure the smartphone alarm rings loudly when the perimeter is breached.

---

## 🚀 Installation & Setup

### 1. Clone the Repository
Download the project files to your local machine:
```bash
git clone [https://github.com/Jejekdf/r-sec-smart-alarm.git](https://github.com/Jejekdf/r-sec-smart-alarm.git)