#include <Arduino.h>

RTC_DATA_ATTR int totalElapsedSeconds = 0; // Cumulative time
RTC_DATA_ATTR int currentSleepSeconds = 20; // Variable sleep

float simulatePhenomenon(int t);

void setup() {
    // 1. Update the time based on how long we JUST slept
    totalElapsedSeconds += currentSleepSeconds;

    // 2. Use totalElapsedSeconds to calculate the "Truth"
    float currentTemp = simulatePhenomenon(totalElapsedSeconds);

    // 3. Determine NEXT sleep duration
    if (currentTemp > 35.0) {
        currentSleepSeconds = 2;
    } else {
        currentSleepSeconds = 20;
    }

    // 4. Sleep
    esp_deep_sleep_start();
}

void loop() {} // Never reached

float simulatePhenomenon(int t) {
  // Simple fire model: starts rising after t
  if (t < 50) return 25.0 + (random(0, 10) / 10.0);
  return 25.0 + (t - 50) * 1.5; // Linear rise
}
