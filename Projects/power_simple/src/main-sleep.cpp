
/*
  ESP32 Deep Sleep Mode Timer Wake UP
 http:://www.electronicwings.com
*/ 

#include <Arduino.h>

#define Time_To_Sleep 5   //Time ESP32 will go to sleep (in seconds)
#define S_To_uS_Factor 1000000ULL      //Conversion factor for micro seconds to seconds 

RTC_DATA_ATTR int bootCount= 0;

void setup() {
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Set timer to 5 seconds
 esp_sleep_enable_timer_wakeup(Time_To_Sleep * S_To_uS_Factor);
  Serial.println("Setup ESP32 to sleep for every " + String(Time_To_Sleep) +
  " Seconds");

  //Go to sleep now
  esp_deep_sleep_start();
 
  Serial.println("This will not print!!"); // This will not get print,as ESP32 goes in Sleep mode.
}

void loop() {} // We don't need loop as ESP32 will initilize each time.