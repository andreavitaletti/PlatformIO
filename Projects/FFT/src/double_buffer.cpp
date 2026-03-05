#include <arduinoFFT.h>

#define SAMPLES 1024
#define SAMPLING_FREQUENCY 2000 
#define TARGET_FREQ 100


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

// Add a second semaphore for the handshake
SemaphoreHandle_t xFFTFinished = NULL;

void TaskSample(void *pvParameters) {
  int64_t next_waketime = esp_timer_get_time();
  const int64_t interval = 1000000 / SAMPLING_FREQUENCY;
  int sampleIdx = 0;

  while (1) {
    float t = (float)sampleIdx / (float)SAMPLING_FREQUENCY;
    fillReal[sampleIdx] = 50.0 * sin(2.0 * M_PI * TARGET_FREQ * t) + 20.0 * sin(2.0 * M_PI * (2*TARGET_FREQ) * t);
    fillImag[sampleIdx] = 0;
    sampleIdx++;

    if (sampleIdx >= SAMPLES) {
      // --- THE HANDSHAKE ---
      // Wait for FFT task to confirm it is finished with procReal/procImag
      // If the FFT is slow, the Sampler will pause here briefly
      if (xFFTFinished != NULL) {
        xSemaphoreTake(xFFTFinished, portMAX_DELAY);
      }

      double *tempReal = fillReal; double *tempImag = fillImag;
      fillReal = procReal; fillImag = procImag;
      procReal = tempReal; procImag = tempImag;

      sampleIdx = 0;
      xSemaphoreGive(xSemaphore); // Tell FFT: "Data is ready"
    }

    next_waketime += interval;
    int64_t sleep_time = next_waketime - esp_timer_get_time();
    if (sleep_time > 0) delayMicroseconds(sleep_time);
    if (sampleIdx % 32 == 0) vTaskDelay(1); 
  }
}

void TaskFFT(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
      // Re-link the library to the newly swapped proc pointers
      ArduinoFFT<double> FFT = ArduinoFFT<double>(procReal, procImag, SAMPLES, SAMPLING_FREQUENCY);

      FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.compute(FFT_FORWARD);
      FFT.complexToMagnitude();

      // Find Max Frequency (Shannon check)
      double threshold = 500.0; // Scaled: Peak * (SAMPLES/something)
      int topBin = 0;
      for (int i = (SAMPLES / 2) - 1; i >= 0; i--) {
        if (procReal[i] > threshold) {
          topBin = i;
          break; 
        }
      }

      double f_max = (topBin * SAMPLING_FREQUENCY) / (double)SAMPLES;
      Serial.printf("Detected Max Freq: %.2f Hz | New Suggested Fs: %.2f Hz\n", f_max, f_max * 2.5);

      // --- THE HANDSHAKE ---
      // Tell the Sampler: "I am done printing, you can have the buffer back"
      xSemaphoreGive(xFFTFinished);
    }
  }
}

void setup() {
  Serial.begin(115200);
  xSemaphore = xSemaphoreCreateBinary();
  xFFTFinished = xSemaphoreCreateBinary();
  
  // Initially, the FFT is "finished" so the Sampler can do the first swap
  xSemaphoreGive(xFFTFinished); 

  xTaskCreatePinnedToCore(TaskSample, "Sampler", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(TaskFFT, "FFT_Task", 10000, NULL, 1, &FFTTaskHandle, 0);
}

void loop() { vTaskDelete(NULL); }