#include <Arduino.h>
#include "arduinoFFT.h"


// Define the DAC and ADC pins
const int adcPin = 36;   // ADC1 (GPIO 34) for reading the sinusoid
int samplingFrequencyADC = 27; // sampling theorem should be at least 2*frequency
const uint16_t samples = 64; 

double vReal[samples];
double vImag[samples];


/* Create FFT object */
ArduinoFFT<double> FFTADC = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequencyADC);

void setup() {
  Serial.begin(115200);
  analogReadResolution(10); 
  analogSetAttenuation(ADC_11db);  // Set ADC attenuation (default 0dB)
}

void loop() {

  for(int i=0; i<samples; i++)
   {
      vReal[i] = analogRead(adcPin)-512;
      vImag[i] = 0;
      /*
      Serial.print(">");
      Serial.print("adc:");    
      Serial.println(analogRead(adcPin)-512);
      */
      delay(round(1.0/samplingFrequencyADC*1000));
  }

  FFTADC.windowing(FFTWindow::Hamming, FFTDirection::Forward);	
  FFTADC.compute(FFTDirection::Forward); 
  FFTADC.complexToMagnitude();
  double x = FFTADC.majorPeak();
  Serial.print("ADC FFT: ");
  Serial.println(x, 6);

  }
