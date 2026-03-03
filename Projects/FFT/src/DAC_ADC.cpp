#include <Arduino.h>
#include <arduinoFFT.h>
#include <driver/dac.h>

#define SAMPLES 1024
#define SAMPLING_FREQUENCY 1000 
#define TARGET_FREQ 20.0

double vReal0[SAMPLES], vImag0[SAMPLES];
double vReal1[SAMPLES], vImag1[SAMPLES];
double *fillReal = vReal0, *fillImag = vImag0;
double *procReal = vReal0, *procImag = vImag0;

volatile int sampleIdx = 0;
double phaseAccum = 0;
const double phaseInc = (2.0 * M_PI * TARGET_FREQ) / (double)SAMPLING_FREQUENCY;

SemaphoreHandle_t xBufferSemaphore;
hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {
    // REDUCED AMPLITUDE: Stay away from 0V and 3.3V rails to avoid clipping
    uint8_t dac_val = (uint8_t)(128 + 60 * sin(phaseAccum)); 
    dac_output_voltage(DAC_CHANNEL_1, dac_val);
    
    /*
    
    The FFT is extremely sensitive to Phase Discontinuity. 
    If your sine wave "jumps" or "stutters" at the end of a buffer, 
    the FFT sees a massive glitch instead of a smooth tone.
    
    Without phaseInc: If you just used a simple loop index, the sine wave would reset to $0$ every 1024 samples. 
    This creates a "click" or "pop" in the data.
    
    With phaseInc: The phaseAccumulator simply keeps adding the increment forever. 
    When it hits $2\pi$, it wraps around. This ensures the last sample of Buffer A and the first sample of 
    Buffer B connect perfectly, creating a seamless, infinite sine wave.
    
    */
    // phaseAccum = 0;
    phaseAccum += phaseInc;
    

    if (phaseAccum >= 2.0 * M_PI) phaseAccum -= 2.0 * M_PI;

    fillReal[sampleIdx] = (double)analogRead(34);
    fillImag[sampleIdx] = 0;
    sampleIdx++;

    if (sampleIdx >= SAMPLES) {
        sampleIdx = 0;
        procReal = fillReal; procImag = fillImag;
        fillReal = (fillReal == vReal0) ? vReal1 : vReal0;
        fillImag = (fillImag == vImag0) ? vImag1 : vImag0;

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xBufferSemaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
    }
}

void TaskFFT(void *pvParameters) {
    ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal0, vImag0, SAMPLES, SAMPLING_FREQUENCY);
    while (1) {
        if (xSemaphoreTake(xBufferSemaphore, portMAX_DELAY) == pdTRUE) {
            
            // --- STEP 1: Remove DC Offset (Crucial to kill that 4Hz peak) ---
            double mean = 0;
            for(int i=0; i<SAMPLES; i++) mean += procReal[i];
            mean /= SAMPLES;
            for(int i=0; i<SAMPLES; i++) procReal[i] -= mean;

            // --- STEP 2: Process FFT ---
            FFT = ArduinoFFT<double>(procReal, procImag, SAMPLES, SAMPLING_FREQUENCY);
            FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.compute(FFT_FORWARD);
            FFT.complexToMagnitude();

            double peak = FFT.majorPeak();
            
            // Log for Serial Plotter (Blue line = Target, Red line = Detected)
            Serial.printf("Target:%f,Detected:%f\n", TARGET_FREQ, peak);
        }
    }
}

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    // ADC Attenuation: 11dB allows reading up to ~3.1V
    analogSetAttenuation(ADC_11db); 
    dac_output_enable(DAC_CHANNEL_1);
    xBufferSemaphore = xSemaphoreCreateBinary();

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000 / SAMPLING_FREQUENCY, true);
    timerAlarmEnable(timer);

    xTaskCreatePinnedToCore(TaskFFT, "FFT", 10000, NULL, 1, NULL, 0);
}

void loop() { vTaskDelete(NULL); }