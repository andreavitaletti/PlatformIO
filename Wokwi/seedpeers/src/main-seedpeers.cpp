#include <Arduino.h>

const int PIN_Q1_SPECTRUM_1 = 19; // PWM Canale 1 (Rosso/Blu)
const int PIN_Q2_SPECTRUM_2 = 18; // PWM Canale 1 (Rosso/Blu)
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

void setLampState(int brightnessQ1, int brightnessQ2) {
  // Con il Core v3.x usi direttamente il numero del PIN
  ledcWrite(PIN_Q1_SPECTRUM_1, brightnessQ1);
  ledcWrite(PIN_Q2_SPECTRUM_2, brightnessQ2);
}

void setup() {
  Serial.begin(115200);

  // Sintassi ESP32 Core v3.x: associa pin, frequenza e risoluzione in un solo passaggio
  ledcAttach(PIN_Q1_SPECTRUM_1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(PIN_Q2_SPECTRUM_2, PWM_FREQ, PWM_RESOLUTION);
  
  setLampState(0,255);
  Serial.println("ready");
}

void loop() {
  Serial.println("ramping up");
  for (int duty = 0; duty <= 255; duty += 5) {
    setLampState(duty, 255-duty);
    Serial.print("duty: ");
    Serial.println(duty);
    delay(20);
  }

  delay(1000);

  Serial.println("ramping down");
  for (int duty = 255; duty >= 0; duty -= 5) {
    setLampState(duty, 255-duty);
    Serial.print("duty: ");
    Serial.println(duty);
    delay(20);
  }
}