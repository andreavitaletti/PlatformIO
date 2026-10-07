#include <Arduino.h>
// Definizione dei PIN
const int PIN_Q1_SPECTRUM_1 = 18; // PWM Canale 1 (Rosso/Blu)
const int PIN_Q2_SPECTRUM_2 = 19; // PWM Canale 2 (Bianco)
// Configurazione del PWM (LEDC) dell'ESP32
const int PWM_FREQ = 5000;
// Frequenza a 5 kHz
const int PWM_RESOLUTION = 8;
// Risoluzione a 8 bit (valori da 0 a 255)
// Canali hardware PWM dell'ESP32 (da 0 a 15 disponibili)
const int CHANNEL_Q1 = 0;
const int CHANNEL_Q2 = 1;
void setup() {
Serial.begin(115200);
// Configurazione del PWM su Arduino Cloud (ESP32 Core v2.x)
ledcSetup(CHANNEL_Q1, PWM_FREQ, PWM_RESOLUTION);
ledcAttachPin(PIN_Q1_SPECTRUM_1, CHANNEL_Q1);
ledcSetup(CHANNEL_Q2, PWM_FREQ, PWM_RESOLUTION);
ledcAttachPin(PIN_Q2_SPECTRUM_2, CHANNEL_Q2);
// Assicuriamoci che i MOSFET siano spenti all'avvio
ledcWrite(CHANNEL_Q1, 0);
ledcWrite(CHANNEL_Q2, 0);
Serial.println("Controller Lampada Piante Avviato!");
}
// Funzione ausiliaria per impostare lo spettro e la luminosità (0 - 255)
void setLampState(int brightnessQ1, int brightnessQ2) {
ledcWrite(CHANNEL_Q1, brightnessQ1);
ledcWrite(CHANNEL_Q2, brightnessQ2);
}
void loop() {
// -------------------------------------------------------------
// MODALITÀ 1: Solo Rosso / Blu (Q1 ON, Q2 OFF)
// -------------------------------------------------------------
Serial.println("Modalità: Solo Rosso / Blu");
for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle += 5) {
setLampState(dutyCycle, 0);
delay(30);
}
delay(2000);
// -------------------------------------------------------------
// MODALITÀ 2: Solo Bianco (Q1 OFF, Q2 ON)
// -------------------------------------------------------------
Serial.println("Modalità: Solo Bianco");
for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle += 5) {
setLampState(0, dutyCycle);
delay(30);
}
delay(2000);
// -------------------------------------------------------------
// MODALITÀ 3: Full Spectrum - Entrambi Accesi (Q1 ON, Q2 ON)
// -------------------------------------------------------------
Serial.println("Modalità: Full Spectrum");
for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle += 5) {
setLampState(dutyCycle, dutyCycle);
delay(30);
}
delay(2000);
// Spegnimento prima di ripartire dal ciclo
setLampState(0, 0);
delay(1000);
}
