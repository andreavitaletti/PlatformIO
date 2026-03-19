#include <Arduino.h>

double drift_multiplier = 1.0;
uint64_t last_raw_micros = 0;
double base_wall_time_s = 0; // The Wall Clock time in seconds (Epoch)

double get_wall_clock() {
    uint64_t current_raw = micros();
    uint64_t elapsed_raw = current_raw - last_raw_micros;
    
    // Convert elapsed microseconds to adjusted seconds
    double elapsed_adjusted_s = (elapsed_raw * drift_multiplier) / 1000000.0;
    return base_wall_time_s + elapsed_adjusted_s;
}

void setup() {
    Serial.begin(115200);
}

void loop() {
    // 1. Send heartbeat to Twin
    static uint32_t last_report = 0;
    if (millis() - last_report > 5000) {
        last_report = millis();
        Serial.print("RAW_MICROS:");
        Serial.print(micros());
        Serial.print(",CUR_WALL:");
        Serial.println(get_wall_clock(), 3); // Tell the twin what we THINK the time is
    }

    // 2. Handle Commands
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        
        // Set the "Starting Line" (Phase)
        if (data.startsWith("SET_PHASE:")) {
            base_wall_time_s = data.substring(10).toDouble();
            last_raw_micros = micros();
            Serial.println("ACK: Phase (Wall Clock) Initialized.");
        }
        
        // Adjust the "Speed" (Frequency)
        if (data.startsWith("SET_MULT:")) {
            // Anchor the time before changing the rate
            base_wall_time_s = get_wall_clock();
            last_raw_micros = micros();
            drift_multiplier = data.substring(9).toDouble();
            Serial.println("ACK: Rate Adjusted.");
        }
    }
}