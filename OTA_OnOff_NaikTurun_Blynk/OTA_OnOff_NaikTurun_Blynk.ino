#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN "" 

#define BLYNK_PRINT Serial 

// --- PENGATURAN WIFI ---
char ssid[] = ""; 
char pass[] = ""; 
// -----------------------

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// ========================================
// LIBRARY UNTUK OTA (OVER-THE-AIR)
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// ========================================

int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 12; 

// --- PENGATURAN INDIKATOR OTA (KEDIP GANDA) ---
#define LED_BUILTIN 2 
unsigned long previousMillis = 0;
const long interval = 1000; // Berkedip tiap 1 detik
int ledState = LOW;
int blynkLedState = 0; // Status untuk LED di aplikasi Blynk
// ----------------------------------------------

const int freq = 30000;
const int resolution = 8;
int dutyCycle = 0;          
bool isMotorRunning = false; 

void applyMotor() {
  if (isMotorRunning) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, dutyCycle);
    
    Blynk.virtualWrite(V1, dutyCycle);
    
    int simulasiRPM = map(dutyCycle, 0, 255, 0, 13000);
    Blynk.virtualWrite(V0, simulasiRPM);
    
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, 0);
  
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V0, 0);
  }
}

BLYNK_WRITE(V3) {
  int buttonState = param.asInt(); 
  if (buttonState == 1) {
    isMotorRunning = true;
    if (dutyCycle == 0) dutyCycle = 150; 
    Serial.println("BLYNK: Motor dinyalakan!");
  } else {
    isMotorRunning = false;
    Serial.println("BLYNK: Motor dimatikan!");
  }
  applyMotor();
}

BLYNK_WRITE(V2) {
  dutyCycle = param.asInt(); 
  Serial.print("BLYNK: Slider diubah ke kecepatan: ");
  Serial.println(dutyCycle);
  
  if (isMotorRunning) {
    applyMotor(); 
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); 
  
  ledcAttach(enable1Pin, freq, resolution);
  
  isMotorRunning = false;
  applyMotor();

  Serial.println("\nMenghubungkan ESP32 ke WiFi dan Server Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Berhasil Terhubung ke Blynk Cloud!");

  // ========================================
  // PENGATURAN AWAL SISTEM OTA
  // ========================================
  ArduinoOTA.setHostname("Robotika-iMCLab"); 

  ArduinoOTA.onStart([]() {
    Serial.println("\n[OTA] Memulai proses update OTA...");
    digitalWrite(LED_BUILTIN, HIGH); 
    Blynk.virtualWrite(V4, 255); // LED Blynk menyala terang saat update
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Update OTA Selesai!");
    digitalWrite(LED_BUILTIN, LOW); 
    Blynk.virtualWrite(V4, 0); 
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]: ", error);
  });
  
  ArduinoOTA.begin();
  Serial.println("[OTA] Sistem OTA Aktif dan Standby!");
  // ========================================
}

void loop() {
  Blynk.run(); 
  ArduinoOTA.handle(); 

  // --- LOGIKA KEDIP GANDA (FISIK & BLYNK) SEBAGAI TANDA STANDBY ---
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // 1. Kedipkan LED Fisik di Robot
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(LED_BUILTIN, ledState);
    
    // 2. Kedipkan LED Virtual di Layar HP (Blynk)
    blynkLedState = (blynkLedState == 0) ? 255 : 0;
    Blynk.virtualWrite(V4, blynkLedState);
  }
}