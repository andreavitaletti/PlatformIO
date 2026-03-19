#include <Arduino.h>

#define DAC_PIN 25 // Uses DAC Channel 1
const int tableSize = 256;
uint8_t sineTable[tableSize];

void setup() {
  // Generate a sine lookup table (0 to 255)
  // We scale it slightly (20-230) to stay in the DAC's linear range
  for (int i = 0; i < tableSize; i++) {
    sineTable[i] = (uint8_t)(127 + 100 * sin(2 * PI * i / tableSize));
  }
}

void loop() {
  for (int i = 0; i < tableSize; i++) {
    dacWrite(DAC_PIN, sineTable[i]);
    delayMicroseconds(100); // Adjust this to change frequency
  }
}