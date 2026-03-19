#include <Arduino.h>
#include "driver/i2s.h"
#include <math.h>

#define SAMPLE_RATE     44100 // 44.1 kHz (CD Quality)
#define SINE_FREQ       440.0 // Standard A4 note
#define I2S_NUM         I2S_NUM_0

void setup() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN); // Enables GPIO25 and 26
}

void loop() {
  static float phase = 0;
  uint16_t sample;
  size_t bytes_written;

  // Generate 1 sample of sine wave
  float val = sin(phase);
  sample = (uint16_t)((val + 1.0) * 127); // Scale -1.0..1.0 to 0..254
  sample <<= 8; // I2S DAC expects high byte

  i2s_write(I2S_NUM, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);

  phase += 2 * PI * SINE_FREQ / SAMPLE_RATE;
  if (phase >= 2 * PI) phase -= 2 * PI;
}