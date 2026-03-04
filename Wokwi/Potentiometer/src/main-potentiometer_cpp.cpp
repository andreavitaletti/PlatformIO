#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Handle for our Queue
QueueHandle_t adcQueue;


// --- ADD THESE PROTOTYPES HERE ---
void TaskReadADC(void *pvParameters);
void TaskSerialPrint(void *pvParameters);


void setup() {
  Serial.begin(115200);

  // Create a queue to hold 5 integers
  adcQueue = xQueueCreate(5, sizeof(int));

  if (adcQueue != NULL) {
    // Create Task for Reading ADC
    // xTaskCreate(TaskReadADC, "ReadADC", 128, NULL, 1, NULL);
    /*
    In the ESP32 implementation of FreeRTOS, 128 (which is 512 bytes) is way too small, 
    especially for TaskSerialPrint. The Serial.println() function uses a significant amount 
    of memory to format strings and manage buffers. When the task runs out of memory, 
    it triggers a "Stack canary watchpoint" or a "Guru Meditation Error," 
    and the ESP32 reboots to protect itself.
    */
    xTaskCreate(TaskReadADC, "ReadADC", 2048, NULL, 1, NULL);
    
    // Create Task for Serial Printing
    xTaskCreate(TaskSerialPrint, "SerialPrint", 2048, NULL, 1, NULL);
  }
}

void loop() {
  // Empty. In FreeRTOS, tasks handle the logic!
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskReadADC(void *pvParameters) {
  int analogPin = 12;
  
  for (;;) {
    int val = analogRead(analogPin);
    
    // Send value to queue (wait 0 ticks if full)
    xQueueSend(adcQueue, &val, 0);
    
    // Non-blocking delay (ticks to wait)
    vTaskDelay(500 / portTICK_PERIOD_MS); 
  }
}

void TaskSerialPrint(void *pvParameters) {
  int receivedVal;
  
  for (;;) {
    // Wait indefinitely until something is in the queue
    if (xQueueReceive(adcQueue, &receivedVal, portMAX_DELAY) == pdPASS) {
      Serial.print("ADC Value: ");
      Serial.println(receivedVal);
    }
  }
}