#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN "" // Sesuaikan dengan yang ada pada Blynk

#define BLYNK_PRINT Serial 

// --- BAGIAN INI SUDAH DISESUAIKAN ---
char ssid[] = ""; // Nama WiFi atau hotspot yang digunakan
char pass[] = ""; // Password WiFi atau hotspot yang sedang digunakan
// ------------------------------------

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 12; 

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
  ledcAttach(enable1Pin, freq, resolution);
  
  isMotorRunning = false;
  applyMotor();

  Serial.println("\nMenghubungkan ESP32 ke WiFi dan Server Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Berhasil Terhubung ke Blynk Cloud!");
}

void loop() {
  Blynk.run(); 
}