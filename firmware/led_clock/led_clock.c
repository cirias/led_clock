/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define DIGITS_NUM       4
#define TICK_DURATION_US 10

// one round fresh all 4 digits once
// duration of one round = TICK_DURATION_US * DIGITS_NUM
/* #define ROUND_PER_COUNT 25000 // count duration = 25000 * 10 * 4 us = 1 sec */
/* #define ROUND_PER_COUNT 2500 */
#define ROUND_PER_COUNT 25 // count duration = 25 * 10 * 4 us = 1 ms

#define FIRST_DIG_GPIO 10
#define FIRST_SEG_GPIO 2

/*
  Our 7 Segment display has pins as follows:

  --A--
  F   B
  --G--
  E   C
  --D--

  By default we are allocating GPIO 2 to segment A, 3 to B etc.
  So, connect GPIO 2 to pin A on the 7 segment LED display etc. Don't forget
  the appropriate resistors, best to use one for each segment!

  Connect button so that pressing the switch connects the GPIO 9 (default) to
  ground (pull down)
*/

int bits[10] = {
  0x3f,  // 0
  0x06,  // 1
  0x5b,  // 2
  0x4f,  // 3
  0x66,  // 4
  0x6d,  // 5
  0x7d,  // 6
  0x07,  // 7
  0x7f,  // 8
  0x6f   // 9
};

int dividers[4] = {
  1,
  10,
  100,
  1000
};

repeating_timer_t timer;
bool tick_callback(repeating_timer_t *rt);
int tick = 0;

int main() {
    stdio_init_all();

    // initialize all pins at once
    uint32_t mask = 0xffu << FIRST_DIG_GPIO | 0xffu << FIRST_SEG_GPIO;
    gpio_init_mask(mask);
    gpio_set_dir_out_masked(mask);

    for (int i = 0; i < 8; i++) {
        // Our bitmap above has a bit set where we need an LED on, BUT, we are pulling low to light
        // so invert our output
        gpio_set_outover(FIRST_SEG_GPIO + i, GPIO_OVERRIDE_INVERT);
    }

    // negative timeout means exact delay (rather than delay between callbacks)
    if (!add_repeating_timer_us(-(TICK_DURATION_US), tick_callback, &tick, &timer)) {
        printf("Failed to add timer\n");
        return 1;
    }

    while (true) {
       tight_loop_contents();
    }
}

// each tick turns exact one digit on for TICK_DURATION_US
bool tick_callback(repeating_timer_t *rt) {
    int* tick = (int*)rt->user_data;

    // the count we are displaying
    int count = *tick / ROUND_PER_COUNT / DIGITS_NUM;

    // the digit we are about to control
    // display even at lower 4 digits, odd at higher 4 digits
    int digit_idx = *tick % DIGITS_NUM;
    int digit_value = (count / dividers[digit_idx]) % 10;

    // display even at lower 4 digits, odd at higher 4 digits
    int digit_pin_idx = digit_idx + (count % 2) * 4;

    // turn off all digit pins
    gpio_clr_mask(0xffu << FIRST_DIG_GPIO);

    // set segments to digit_value
    gpio_put_masked(0xffu << FIRST_SEG_GPIO, bits[digit_value] << FIRST_SEG_GPIO);

    // turn on the current digit
    gpio_put_masked(0xffu << FIRST_DIG_GPIO, (1u << digit_pin_idx) << FIRST_DIG_GPIO);

    *tick = ((*tick) + 1) % (DIGITS_NUM * ROUND_PER_COUNT * 10000);

    return true; // keep repeating
}
