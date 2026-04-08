#include <arduinoFFT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"

#define SAMPLES 1024
#define SAMPLING_FREQUENCY 2000 
#define TARGET_FREQ 100

// Wi-Fi credentials
// const char* ssid = "YOUR_SSID";
// const char* password = "YOUR_PASSWORD";

// Raw URL to the data gerenreated by generate_github_data.py 
// e.g. https://raw.githubusercontent.com/user/repo/main/buffer_data.txt
const char* dataUrl = "https://raw.githubusercontent.com/andreavitaletti/PlatformIO/refs/heads/main/Projects/FFT/buffer_data.txt";

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

void TaskDownload(void *pvParameters) {
  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  // taken from secrets.h
  /*
  #ifndef SECRETS_H
  #define SECRETS_H

  const char* ssid = "YOUR_ACTUAL_SSID_HERE";
  const char* password = "YOUR_ACTUAL_PASSWORD_HERE";

  #endifsword);
  
  */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");

  while (1) {
    // Check if Wi-Fi is still connected
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure *client = new WiFiClientSecure;
      if (client) {
        // Skip SSL certificate validation for simplicity.
        // In extreme production environments, consider providing the root CA instead.
        client->setInsecure(); 

        HTTPClient http;
        Serial.println("Starting HTTP GET...");
        if (http.begin(*client, dataUrl)) {
          int httpCode = http.GET();
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            
            int sampleIdx = 0;
            int startIdx = 0;
            int payloadLen = payload.length();

            while (startIdx < payloadLen && sampleIdx < SAMPLES) {
                // Find next separator
                int nextSep = -1;
                for (int i = startIdx; i < payloadLen; i++) {
                    char c = payload.charAt(i);
                    if (c == '\n' || c == '\r' || c == ',') {
                        nextSep = i;
                        break;
                    }
                }
                
                if (nextSep == -1) nextSep = payloadLen;

                if (nextSep > startIdx) {
                    String valStr = payload.substring(startIdx, nextSep);
                    valStr.trim();
                    if (valStr.length() > 0) {
                        fillReal[sampleIdx] = valStr.toDouble();
                        fillImag[sampleIdx] = 0;
                        sampleIdx++;
                    }
                }
                startIdx = nextSep + 1;
            }

            Serial.printf("Downloaded %d samples\n", sampleIdx);

            if (sampleIdx == SAMPLES) {
              // --- THE HANDSHAKE ---
              // Wait for FFT task to confirm it is finished with procReal/procImag
              if (xFFTFinished != NULL) {
                xSemaphoreTake(xFFTFinished, portMAX_DELAY);
              }

              double *tempReal = fillReal; double *tempImag = fillImag;
              fillReal = procReal; fillImag = procImag;
              procReal = tempReal; procImag = tempImag;

              xSemaphoreGive(xSemaphore); // Tell FFT: "Data is ready"
            } else {
              Serial.println("Not enough samples downloaded.");
            }
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }
          http.end();
        } else {
            Serial.println("[HTTP] Unable to connect");
        }
        delete client;
      } else {
          Serial.println("Unable to create client");
      }
    } else {
      Serial.println("Wi-Fi Disconnected. Reconnecting...");
      WiFi.reconnect();
    }
    
    // Download again every 5 seconds (adjust this to throttle how often you pull from GitHub)
    vTaskDelay(5000 / portTICK_PERIOD_MS); 
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

  // Required larger stack size 32768 for WiFi and HTTP Client (SSL often needs plenty of memory)
  xTaskCreatePinnedToCore(TaskDownload, "DownloadTask", 32768, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(TaskFFT, "FFT_Task", 10000, NULL, 1, &FFTTaskHandle, 0);
}

void loop() { vTaskDelete(NULL); }
