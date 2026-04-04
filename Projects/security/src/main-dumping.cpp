// ESP32 RECEIVER CODE
#include <Arduino.h>

const char* key = "SECRET_12345";

void setup() {
    Serial.begin(115200);
    // If you do not use the constant key the compiler remove it
    Serial.println(key); // Force the compiler to keep the string
}

void loop() {
  // Key is just sitting in memory
}
