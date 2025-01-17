#include <Arduino.h>

// Define the DAC and ADC pins
const int dacPin = 25;   // DAC1 (GPIO 25) for sinusoid output


// Parameters for the sine wave
const int amplitude = 100;   // Amplitude of the sine wave (max 255 for 8-bit DAC)
const int offset = 128;      // DC offset (middle of the DAC range)
const float signalFrequency = 5.0;  // Frequency of the sine wave in Hz
int samplingFrequencyDAC = 1000; // sampling theorem should be at least 2*frequency




void setup() {
  Serial.begin(115200);

  // Initialize DAC pin (GPIO 25)
  dacWrite(dacPin, 0);  // Initialize DAC with a low value

}

void loop() {
      for (int i = 0; i < samplingFrequencyDAC; i++) {
      int sineValue = (int)(amplitude * sin(2.0 * PI * signalFrequency * i / samplingFrequencyDAC) + offset);
      dacWrite(dacPin, sineValue);  // Write to DAC (8-bit value)
      /*
      Serial.print(">");
      Serial.print("dac:");    
      Serial.println(sineValue);
      */
      delay(round(1.0/samplingFrequencyDAC*1000));
      } 
    
}
