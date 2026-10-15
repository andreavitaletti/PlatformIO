// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
// SPDX-License-Identifier: MIT
// Copyright 2023 

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t sig;
  pin_t out_minus;
} chip_state_t;

static void on_sig_change(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = (chip_state_t *)user_data;
  // MOSFET low-side switch: SIG HIGH = drain connects to source (GND)
  pin_write(chip->out_minus, value ? LOW : HIGH);
}

void chip_init(void) {

  printf("init \n");
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  chip->sig       = pin_init("SIG",       INPUT);
  pin_init("GND_IN",                      INPUT);
  pin_init("V_PLUS",                      INPUT);
  pin_init("V_MINUS",                     INPUT);
  pin_init("OUT_PLUS",                    INPUT);
  chip->out_minus = pin_init("OUT_MINUS", OUTPUT_LOW);

  // Initialize output to match current SIG state
  pin_write(chip->out_minus, pin_read(chip->sig) ? LOW : HIGH);

  const pin_watch_config_t watch_cfg = {
    .edge       = BOTH,
    .pin_change = on_sig_change,
    .user_data  = chip,
  };
  pin_watch(chip->sig, &watch_cfg);
}
