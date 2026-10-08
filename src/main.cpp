/**
 * @file main.cpp
 * @brief Project Sentinel - R-SEC System Smart Alarm
 * 
 * ESP32-based laser tripwire security system.
 * Monitors perimeter using LDR sensor, delivers local audio-visual alerts,
 * and synchronizes with Blynk IoT platform with rate-limited telemetry
 * and a 3-state anti-spam machine.
 */

#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>

// =========================================================================
// HARDWARE PERIPHERALS & TIMERS
// =========================================================================
LiquidCrystal_I2C lcd(0x27, 16, 2); 
BlynkTimer timer;                   

// GPIO Definitions
const int ldrPin    = 32;   // Analog In: LDR laser receiver (divider w/ 10k resistor)
const int buzzerPin = 4;    // Digital Out: Passive buzzer (tone output)
const int ledPin    = 27;   // Digital Out: Red visual alarm LED

// =========================================================================
// SYSTEM CONFIGURATION & STATE MACHINE
// =========================================================================
const int safeThreshold          = 1500;  // Analog threshold: < 1500 indicates interrupted laser
const unsigned long alarmDuration = 7000; // Alarm hold duration in milliseconds (7 seconds)

enum SystemState {
  STATE_SECURE,     // Normal monitoring, perimeter clear
  STATE_ALARM,      // Intrusion detected, 7-second audio/visual siren latch
  STATE_WAIT_CLEAR  // Latch elapsed but laser still blocked (prevents notification spam)
};

SystemState currentState = STATE_SECURE;
unsigned long alarmTriggerTime = 0;
bool isOnline = false;
int lastDisplayedLight = -1;

// =========================================================================
// AUDIO ASSETS (Frequencies in Hz)
// =========================================================================
#define NOTE_E5  659   
#define NOTE_G5  784   
#define NOTE_A5  880   
#define NOTE_B5  988   
#define NOTE_C6  1047  
#define NOTE_D6  1175  
#define NOTE_F6  1397  
#define NOTE_A6  1760  

/**
 * @brief Updates LCD display for secure standby state without screen flicker.
 */
void updateSecureDisplay(int lightValue, bool forceUpdate = false) {
  if (currentState != STATE_SECURE) return;

  if (forceUpdate || abs(lightValue - lastDisplayedLight) > 30) {
    lastDisplayedLight = lightValue;
    lcd.setCursor(0, 0);
    lcd.print("Sistem Aman...  ");
    lcd.setCursor(0, 1);
    lcd.print("Sensor: ");
    lcd.print(lightValue);
    lcd.print("     ");
  }
}

/**
 * @brief High-frequency sensor checking & state machine execution (10Hz).
 * Edge-triggered alarm prevents redundant notifications and cloud flooding.
 */
void checkSensor() {
  int lightValue = analogRead(ldrPin);

  switch (currentState) {
    case STATE_SECURE:
      if (lightValue < safeThreshold) {
        // Laser interrupted: transition to ALARM
        currentState = STATE_ALARM;
        alarmTriggerTime = millis();

        // Local alert display
        lcd.setCursor(0, 0);
        lcd.print("AWAS PENYUSUP!  ");
        lcd.setCursor(0, 1);
        lcd.print("Sinar Terpotong!");

        // Cloud notification (edge-triggered once)
        if (isOnline) {
          Blynk.virtualWrite(V1, "AWAS PENYUSUP!");
          Blynk.virtualWrite(V3, 1);
          Blynk.logEvent("peringatan_bahaya", "Awas! Ada penyusup terdeteksi di area rumah!");
        }
      } else {
        updateSecureDisplay(lightValue);
      }
      break;

    case STATE_ALARM:
      if (millis() - alarmTriggerTime < alarmDuration) {
        // Pulsing audio-visual alarm (150ms cadence)
        bool pulse = (millis() / 150) % 2 == 0;
        digitalWrite(ledPin, pulse ? HIGH : LOW);
        if (pulse) {
          tone(buzzerPin, 2400);
        } else {
          noTone(buzzerPin);
        }
      } else {
        // Alarm duration complete; shut off siren
        noTone(buzzerPin);
        digitalWrite(ledPin, LOW);

        // Anti-spam check: check if laser is still blocked
        if (lightValue < safeThreshold) {
          currentState = STATE_WAIT_CLEAR;
          lcd.setCursor(0, 0);
          lcd.print("Sinar Terhalang!");
          lcd.setCursor(0, 1);
          lcd.print("Menunggu Clear..");
          if (isOnline) {
            Blynk.virtualWrite(V1, "Sinar Terhalang!");
          }
        } else {
          currentState = STATE_SECURE;
          updateSecureDisplay(lightValue, true);
          if (isOnline) {
            Blynk.virtualWrite(V1, "Sistem Aman...");
            Blynk.virtualWrite(V3, 0);
          }
        }
      }
      break;

    case STATE_WAIT_CLEAR:
      // Wait until laser beam is fully restored before re-arming
      if (lightValue >= safeThreshold) {
        currentState = STATE_SECURE;
        updateSecureDisplay(lightValue, true);
        if (isOnline) {
          Blynk.virtualWrite(V1, "Sistem Aman...");
          Blynk.virtualWrite(V3, 0);
        }
      }
      break;
  }
}

/**
 * @brief Streams raw analog sensor data to Blynk gauge/chart at a safe 1Hz rate.
 * Prevents triggering Blynk cloud rate limit / flood protection.
 */
void sendTelemetry() {
  if (isOnline && Blynk.connected()) {
    int lightValue = analogRead(ldrPin);
    Blynk.virtualWrite(V2, lightValue);
  }
}

/**
 * @brief Non-blocking background health check for WiFi and Blynk connection.
 */
void checkConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!Blynk.connected()) {
      Blynk.connect(1000);
    }
    isOnline = Blynk.connected();
  } else {
    isOnline = false;
    WiFi.reconnect();
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize display
  lcd.init();
  lcd.backlight();

  // Initialize outputs
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);

  // 1. Boot Splash Screen
  lcd.setCursor(0, 0);
  lcd.print(" PROJECT SENTINEL");
  lcd.setCursor(0, 1);
  lcd.print(" BOOTING OS...  ");
  delay(1200);

  // 2. Hardware Diagnostics Animation
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" R-SEC SYSTEM   ");
  lcd.setCursor(0, 1);

  for (int i = 0; i < 16; i++) {
    lcd.print(">");
    digitalWrite(ledPin, HIGH);
    tone(buzzerPin, 1800, 30);
    delay(40);
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
    delay(50);
  }

  // 3. Startup Melody
  int melody[] = {
    NOTE_E5, NOTE_G5, NOTE_C6, 
    NOTE_A5, NOTE_C6, NOTE_F6, 
    NOTE_B5, NOTE_B5, NOTE_C6, NOTE_D6, NOTE_C6,
    NOTE_E5, NOTE_G5, NOTE_C6,
    NOTE_A5, NOTE_C6, NOTE_F6, NOTE_A6,
    NOTE_B5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_D6, NOTE_C6
  };

  int noteDurations[] = {
    200, 200, 450, 200, 200, 450, 150, 150, 150, 150, 500,
    200, 200, 450, 200, 200, 200, 450, 150, 150, 150, 150, 150, 650
  };

  for (int i = 0; i < 24; i++) {
    digitalWrite(ledPin, HIGH);
    tone(buzzerPin, melody[i], noteDurations[i]);
    int pauseBetweenNotes = noteDurations[i] * 1.35;
    delay(pauseBetweenNotes);
    digitalWrite(ledPin, LOW);
  }
  noTone(buzzerPin);

  // 4. Hybrid Network Initialization
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mencari WiFi... ");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int timeoutCounter = 0;
  while (WiFi.status() != WL_CONNECTED && timeoutCounter < 20) {
    delay(500);
    timeoutCounter++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect(4000);
    isOnline = Blynk.connected();

    lcd.setCursor(0, 0);
    lcd.print("  SYSTEM READY  ");
    lcd.setCursor(0, 1);
    lcd.print(isOnline ? " IoT Terkoneksi " : "WiFi OK/BlynkOff");
  } else {
    isOnline = false;
    lcd.setCursor(0, 0);
    lcd.print("  SYSTEM READY  ");
    lcd.setCursor(0, 1);
    lcd.print("Mode Offline(OK)");
  }
  delay(1500);
  lcd.clear();

  // Attach recurring tasks to non-blocking timer
  timer.setInterval(100L, checkSensor);       // 10Hz polling for instant tripwire response
  timer.setInterval(1000L, sendTelemetry);    // 1Hz telemetry to Blynk to prevent flood
  timer.setInterval(30000L, checkConnection); // 30s auto-reconnect checks
}

void loop() {
  if (isOnline) {
    Blynk.run();
  }
  timer.run();
}