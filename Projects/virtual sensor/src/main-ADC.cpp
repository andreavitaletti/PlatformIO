// ESP32 RECEIVER CODE
#include <Arduino.h>

#define ADC_PIN 34 // ADC1_CH6

void setup() {
  Serial.begin(115200);
  // Set attenuation to 11dB (allows 0V - 3.1V range)
  analogSetAttenuation(ADC_11db); 
}

void loop() {
  int rawValue = analogRead(ADC_PIN); // from 0 to 4095  
  // Print to Serial Plotter
  Serial.println(rawValue);
  delayMicroseconds(500); // Sample rate control
}