#define BLYNK_TEMPLATE_ID "TMPL6l2QFpjEM"
#define BLYNK_TEMPLATE_NAME "Robotik1"
#define BLYNK_AUTH_TOKEN "Ai-2TAtgYK_dhP689-zoxOjfyglgtNaO"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>

// WiFi Hotspot
char ssid[] = "zxcvbnm";
char pass[] = "zzzzzzzzz";

// Pin ESP32
const int ledPin = 2;
const int motor1Pin1 = 27;
const int motor1Pin2 = 26;
const int enable1Pin = 12;

// Pin sensor RPM
const int sensorRPM = 25;

// Pengaturan PWM ESP32 Core 3.3.12
const int freq = 30000;
const int resolution = 8;

int dutyCycle = 200;
const int DUTY_MIN = 0;
const int DUTY_MAX = 255;

// Batas RPM
const int RPM_MIN = 0;
const int RPM_MAX = 13000;

bool motorOn = false;
volatile unsigned long pulseCount = 0;

unsigned long lastRPMTime = 0;
int rpmMotor = 0;

const int PULSES_PER_REVOLUTION = 1;

BlynkTimer timer;

// Menghitung pulsa sensor (Interrupt)
void IRAM_ATTR hitungPulsa() {
  pulseCount++;
}

// Mengatur motor dan LED
void applyMotor() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  if (motorOn && dutyCycle > 0) {
    ledcWrite(enable1Pin, dutyCycle);
  } else {
    ledcWrite(enable1Pin, 0);
  }

  digitalWrite(
    ledPin,
    motorOn && dutyCycle > 0 ? HIGH : LOW
  );
}

// Menyalakan motor
void MotorOn() {
  if (dutyCycle == 0) {
    dutyCycle = 200;
  }

  motorOn = true;
  applyMotor();

  Serial.println("Motor ON");
}

// Mematikan motor
void MotorOff() {
  motorOn = false;
  applyMotor();

  Serial.println("Motor OFF");
}

// Slider PWM pada V2
BLYNK_WRITE(V2) {
  dutyCycle = constrain(
    param.asInt(),
    DUTY_MIN,
    DUTY_MAX
  );

  applyMotor();

  Blynk.virtualWrite(V1, dutyCycle);

  Serial.print("PWM: ");
  Serial.println(dutyCycle);
}

// Tombol ON/OFF pada V3
BLYNK_WRITE(V3) {
  int status = param.asInt();

  if (status == 1) {
    MotorOn();
  } else {
    MotorOff();
  }
}

// Mengukur RPM setiap 1 detik
void updateRPM() {
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - lastRPMTime;

  if (elapsed >= 1000) {
    unsigned long pulses;

    noInterrupts();
    pulses = pulseCount;
    pulseCount = 0;
    interrupts();

    if (motorOn && PULSES_PER_REVOLUTION > 0) {
      rpmMotor = (int)(
        (pulses * 60000.0) /
        (PULSES_PER_REVOLUTION * elapsed)
      );
    } else {
      rpmMotor = 0;
    }

    rpmMotor = constrain(
      rpmMotor,
      RPM_MIN,
      RPM_MAX
    );

    lastRPMTime = currentTime;

    // Kirim data ke Blynk
    Blynk.virtualWrite(V0, rpmMotor);
    Blynk.virtualWrite(V1, dutyCycle);

    Serial.print("RPM: ");
    Serial.print(rpmMotor);
    Serial.print(" | PWM: ");
    Serial.println(dutyCycle);
  }
}

// Saat Blynk terhubung
BLYNK_CONNECTED() {
  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V1, dutyCycle);
  Blynk.virtualWrite(V2, dutyCycle);
  Blynk.virtualWrite(V3, motorOn ? 1 : 0);
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(sensorRPM, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(sensorRPM),
    hitungPulsa,
    FALLING
  );

  // PWM ESP32 Core 3.3.12
  if (!ledcAttach(enable1Pin, freq, resolution)) {
    Serial.println("Gagal mengatur PWM!");

    while (true) {
      delay(1000);
    }
  }

  motorOn = false;
  applyMotor();

  // Koneksi ke hotspot dan Blynk
  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  // Inisialisasi OTA
  ArduinoOTA.setHostname("ESP32-Robotik1"); // Nama port OTA di Arduino IDE
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    Serial.println("Memulai update OTA: " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nUpdate OTA Selesai!");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Proses: %u%%\r\n", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Gagal");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Gagal Memulai");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Gagal Koneksi");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Gagal Terima Data");
    else if (error == OTA_END_ERROR) Serial.println("Gagal Mengakhiri");
  });

  ArduinoOTA.begin();

  lastRPMTime = millis();

  // Update RPM dan PWM setiap 1 detik
  timer.setInterval(1000L, updateRPM);

  Serial.println("ESP32 + Blynk + OTA siap!");
}

void loop() {
  Blynk.run();
  timer.run();
  ArduinoOTA.handle(); // Menangani proses update OTA
}
