
 // ============================================================
 // Kontrol Motor DC + LED dengan PWM via Serial
 // ESP32 Arduino Core 3.3.12
 //
 // Perintah Serial Monitor:
 // '1'       -> Motor ON
 // '0'       -> Motor OFF
 // '+'       -> Naikkan kecepatan (+10)
 // '-'       -> Turunkan kecepatan (-10)
 // 'PWM:xxx' -> Atur PWM langsung (0-255)
 // ============================================================

const int ledPin = 2;

const int motor1Pin1 = 27;
const int motor1Pin2 = 26;
const int enable1Pin = 12;

// Pengaturan PWM ESP32 Core 3.x
const int freq = 30000;
const int resolution = 8;

int dutyCycle = 200;
const int DUTY_STEP = 10;
const int DUTY_MIN = 0;
const int DUTY_MAX = 255;

String inputString = "";

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);

  // Konfigurasi PWM untuk ESP32 Core 3.3.12
  if (!ledcAttach(enable1Pin, freq, resolution)) {
    Serial.println("Gagal mengatur PWM!");
    while (true) {
      delay(1000);
    }
  }

  // Atur arah motor dan pastikan motor mati
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  ledcWrite(enable1Pin, 0);
  digitalWrite(ledPin, LOW);

  Serial.println("ESP32 Motor Controller siap.");
  Serial.println("1=ON, 0=OFF, +=naik, -=turun, PWM:xxx=set");
}

void applyMotor() {
  // Arah putaran motor
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  // Atur PWM berdasarkan pin pada Core 3.x
  ledcWrite(enable1Pin, dutyCycle);

  // LED indikator
  digitalWrite(ledPin, dutyCycle > 0 ? HIGH : LOW);
}

void MotorOn() {
  if (dutyCycle == 0) {
    dutyCycle = 200;
  }

  applyMotor();

  Serial.print("Motor ON, duty = ");
  Serial.println(dutyCycle);
}

void MotorOff() {
  dutyCycle = 0;
  applyMotor();

  Serial.println("Motor OFF, duty = 0");
}

void naikkanKecepatan() {
  dutyCycle += DUTY_STEP;
  dutyCycle = constrain(dutyCycle, DUTY_MIN, DUTY_MAX);

  applyMotor();

  Serial.print("Kecepatan NAIK, duty = ");
  Serial.println(dutyCycle);
}

void turunkanKecepatan() {
  dutyCycle -= DUTY_STEP;
  dutyCycle = constrain(dutyCycle, DUTY_MIN, DUTY_MAX);

  applyMotor();

  Serial.print("Kecepatan TURUN, duty = ");
  Serial.println(dutyCycle);
}

void setDuty(int val) {
  dutyCycle = constrain(val, DUTY_MIN, DUTY_MAX);

  applyMotor();

  Serial.print("Set duty = ");
  Serial.println(dutyCycle);
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      inputString.trim();

      if (inputString.startsWith("PWM:")) {
        String angka = inputString.substring(4);

        // Pastikan nilai PWM berupa angka
        bool valid = angka.length() > 0;

        for (unsigned int i = 0; i < angka.length(); i++) {
          if (!isDigit(angka[i])) {
            valid = false;
            break;
          }
        }

        if (valid) {
          setDuty(angka.toInt());
        } else {
          Serial.println("Format salah. Contoh: PWM:150");
        }
      }
      else if (inputString.length() > 0) {
        char cmd = inputString.charAt(0);

        if (cmd == '1') {
          MotorOn();
        }
        else if (cmd == '0') {
          MotorOff();
        }
        else if (cmd == '+') {
          naikkanKecepatan();
        }
        else if (cmd == '-') {
          turunkanKecepatan();
        }
        else {
          Serial.print("Perintah tidak dikenal: ");
          Serial.println(inputString);
        }
      }

      inputString = "";
    }
    else {
      inputString += c;
    }
  }
}
