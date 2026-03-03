#include <arduinoFFT.h>
#include <math.h>

#define SAMPLES 1024
#define SAMPLING_FREQUENCY 1000 // Reduced for stability during testing

double vReal[SAMPLES];
double vImag[SAMPLES];
TaskHandle_t FFTTaskHandle = NULL;
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);


void TaskSample(void *pvParameters) {
  // Use a lower priority or ensure we don't starve the IDLE task
  const double TARGET_FREQ = 100.0;
  const double AMPLITUDE = 50.0;

  while (1) {
    for (int i = 0; i < SAMPLES; i++) {
      // Calculate synthetic sine
      float time = (float)i / (float)SAMPLING_FREQUENCY;
      vReal[i] = AMPLITUDE * sin(2.0 * M_PI * TARGET_FREQ * time);
      // vReal[i] = analogRead(34); // Read from GPIO 34
      vImag[i] = 0;
      
      // CRITICAL: Small delay to prevent Watchdog Trigger
      // If SAMPLING_FREQUENCY is low, this works fine.
      vTaskDelay(pdMS_TO_TICKS(1)); 
    }
    
    xTaskNotifyGive(FFTTaskHandle);
    
    // Give the CPU a breather between buffers
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void TaskFFT(void *pvParameters) {
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();

    Serial.printf("Peak: %.2f Hz\n", FFT.majorPeak());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to start

  // Increased stack size to 8192 to prevent Stack Overflow
  xTaskCreatePinnedToCore(TaskSample, "Sampler", 8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskFFT, "FFT_Proc", 8192, NULL, 1, &FFTTaskHandle, 0);
}

void loop() {
  vTaskDelete(NULL);
}