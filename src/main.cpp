/**
 * @file main.cpp
 * @brief Project Sentinel - IoT Perimeter Smart Alarm
 * * An ESP32-based laser tripwire security system. It monitors a perimeter 
 * using an LDR sensor and provides local audio-visual feedback alongside 
 * real-time remote monitoring and push notifications via Blynk IoT.
 * Features a hybrid network failsafe to maintain offline hardware alarms 
 * if the WiFi connection fails.
 */

// =========================================================================
// CLOUD CONFIGURATION (Blynk IoT)
// Note: Keep the Auth Token secure and do not share it publicly.
// =========================================================================
#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN_HERE" 

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>

// =========================================================================
// NETWORK CONFIGURATION
// =========================================================================
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// =========================================================================
// HARDWARE PERIPHERALS & TIMERS
// =========================================================================
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 I2C Display at address 0x27
BlynkTimer timer;                   // Hardware-agnostic timer for polling

// Pin Definitions
const int ldrPin = 32;     // Analog input for laser intensity
const int buzzerPin = 4;   // Digital output for the active buzzer
const int ledPin = 27;     // Digital output for the physical alarm LED

// =========================================================================
// SYSTEM STATE VARIABLES
// =========================================================================
int safeThreshold = 1500;        // Trigger point: Values below this indicate a broken beam
bool isOnline = false;           // Tracks if the system successfully connected to Blynk
bool notificationSent = false;   // Anti-spam flag to ensure only one alert per event

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
 * @brief Core sensor polling routine.
 * Reads the LDR value, determines the security state, updates local hardware,
 * and syncs data to the cloud dashboard.
 */
void checkSensor() {
  int lightValue = analogRead(ldrPin);
  
  // Constantly stream raw sensor data to the mobile dashboard graph
  if (isOnline) {
    Blynk.virtualWrite(V2, lightValue); 
  }
  
  // --- STATE: INTRUSION DETECTED ---
  if (lightValue < safeThreshold) {
    // Trigger local hardware alarms
    digitalWrite(buzzerPin, HIGH); 
    digitalWrite(ledPin, HIGH);    
    
    // Update local display
    lcd.setCursor(0, 0);
    lcd.print("AWAS PENYUSUP!  "); 
    lcd.setCursor(0, 1);
    lcd.print("Sinar Terpotong!");
    
    // Cloud synchronization and push notification
    if (isOnline) {
      Blynk.virtualWrite(V1, "AWAS PENYUSUP!"); 
      Blynk.virtualWrite(V3, 1); // Turn ON virtual dashboard LED
      
      // Dispatch alert event only if it hasn't been sent yet (Anti-Spam)
      if (!notificationSent) {
        Blynk.logEvent("peringatan_bahaya", "Awas! Ada penyusup terdeteksi di area rumah!"); 
        notificationSent = true; 
      }
    }
    
  } 
  // --- STATE: PERIMETER SECURE ---
  else {
    // Silence local hardware alarms
    digitalWrite(buzzerPin, LOW);  
    digitalWrite(ledPin, LOW);     
    
    // Update local display with real-time sensor readout
    lcd.setCursor(0, 0);
    lcd.print("Sistem Aman...  ");
    lcd.setCursor(0, 1);
    lcd.print("Sensor: ");
    lcd.print(lightValue);
    lcd.print("    "); // Pad with spaces to clear residual characters
    
    // Reset cloud status and anti-spam lock
    if (isOnline) {
      Blynk.virtualWrite(V1, "Sistem Aman..."); 
      Blynk.virtualWrite(V3, 0); // Turn OFF virtual dashboard LED
      
      if (notificationSent) {
        notificationSent = false; 
      }
    }
  }
}

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize physical alarm pins
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT); 

  // 1. Display boot splash screen
  lcd.setCursor(0, 0);
  lcd.print(" PROJECT SENTINEL");
  lcd.setCursor(0, 1);
  lcd.print(" BOOTING OS...  ");
  delay(1500);

  // 2. Execute hardware diagnostics animation (Progress bar effect)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" R-SEC SYSTEM   ");
  lcd.setCursor(0, 1);
  
  for(int i = 0; i < 16; i++) {
    lcd.print(">"); 
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    delay(40); 
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    delay(80);
  }

  // 3. Play startup melody sequence
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
  
  WiFi.begin(ssid, pass);
  
  // Wait for connection with a hardcoded timeout (~10 seconds)
  int timeoutCounter = 0;
  while (WiFi.status() != WL_CONNECTED && timeoutCounter < 20) { 
    delay(500);
    timeoutCounter++;
  }

  // Determine operational mode based on WiFi success
  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    // Online Mode: Connect to Blynk Cloud
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    isOnline = true;
    
    lcd.setCursor(0, 0);
    lcd.print("  SYSTEM READY  ");
    lcd.setCursor(0, 1);
    lcd.print(" IoT Terkoneksi ");
  } else {
    // Offline Mode: Proceed with local hardware tracking only
    isOnline = false;
    
    lcd.setCursor(0, 0);
    lcd.print("  SYSTEM READY  ");
    lcd.setCursor(0, 1);
    lcd.print("Mode Offline(OK)");
  }
  delay(2000);
  lcd.clear();
  
  // Attach the sensor routine to the timer (Executes every 200ms / 5Hz)
  timer.setInterval(200L, checkSensor);
}

void loop() {
  // Only execute cloud background tasks if connected
  if (isOnline) {
    Blynk.run(); 
  }
  
  // Maintain local hardware polling
  timer.run(); 
}