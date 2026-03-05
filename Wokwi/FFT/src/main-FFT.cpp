#include <arduinoFFT.h>

#define SAMPLES 1024
#define SAMPLING_FREQUENCY 500
#define TARGET_FREQ 1 // smal frequency to allow the simulator to work properly


// 1. Static allocation (prevents Stack Overflow)
double vReal0[SAMPLES], vImag0[SAMPLES];
double vReal1[SAMPLES], vImag1[SAMPLES];

// Control pointers
double *fillReal = vReal0;
double *fillImag = vImag0;
double *procReal = vReal1;
double *procImag = vImag1;

volatile bool bufferReady = false;
SemaphoreHandle_t xSemaphore = NULL;
TaskHandle_t FFTTaskHandle = NULL;

// 2. Optimized Sampling Task
void TaskSample(void *pvParameters) {
  int64_t next_waketime = esp_timer_get_time();
  const int64_t interval = 1000000 / SAMPLING_FREQUENCY; // in microseconds
  int sampleIdx = 0;

  while (1) {
    // Generate synthetic signal
    float t = (float)sampleIdx / (float)SAMPLING_FREQUENCY;
    fillReal[sampleIdx] = 50.0 * sin(2.0 * M_PI * TARGET_FREQ * t);
    fillImag[sampleIdx] = 0;
    sampleIdx++;

    if (sampleIdx >= SAMPLES) {
      // Swap pointers safely
      double *tempReal = fillReal;
      double *tempImag = fillImag;
      
      fillReal = procReal;
      fillImag = procImag;
      
      procReal = tempReal;
      procImag = tempImag;

      sampleIdx = 0;
      // Signal the FFT task
      xSemaphoreGive(xSemaphore);
    }

    // High precision microsecond delay
    next_waketime += interval;
    int64_t sleep_time = next_waketime - esp_timer_get_time();
    if (sleep_time > 0) {
      delayMicroseconds(sleep_time);
    }
    
    // Periodically allow the IDLE task to reset the Watchdog
    if (sampleIdx % 32 == 0) {
      vTaskDelay(1); 
    }
  }
}

// 3. Optimized FFT Task
void TaskFFT(void *pvParameters) {
  // Initialize FFT object once
  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal0, vImag0, SAMPLES, SAMPLING_FREQUENCY);

  while (1) {
    // Wait for the semaphore
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
      // Point the FFT to the buffer we just filled
      // Note: We use the pointer 'procReal' which was swapped in TaskSample
      FFT = ArduinoFFT<double>(procReal, procImag, SAMPLES, SAMPLING_FREQUENCY);

      FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.compute(FFT_FORWARD);
      FFT.complexToMagnitude();

      Serial.printf("Peak: %.2f Hz\n", FFT.majorPeak());
    }
  }
}

void setup() {
  Serial.begin(115200);
  xSemaphore = xSemaphoreCreateBinary();

  // Core 1: Sampling (High Priority)
  xTaskCreatePinnedToCore(TaskSample, "Sampler", 4096, NULL, 5, NULL, 1);
  
  // Core 0: FFT (Lower Priority, Huge Stack for Math)
  xTaskCreatePinnedToCore(TaskFFT, "FFT_Task", 10000, NULL, 1, &FFTTaskHandle, 0);
}

void loop() { vTaskDelete(NULL); }