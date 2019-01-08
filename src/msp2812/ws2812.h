#ifndef WS2812_H
#define WS2812_H

/* Uses:
 *     UCB0 (using SPI MOSI as ws2812 waveform)
 *     SMCLK as clocksource for spi
 *     DMA0
 *     TA1 400Hz interval timer with interrupt to generate frames at 400FPS
 *     ACLK as interval timer clock source
 *
 * Assumes:
 *     SMCLK clocked at ~2.4MHz
 *     ACLK  clocked at ~32.768kHz
 */

#include <stdint.h>

#include "config.h"

#define DITHER_FRAMES (16) /* 400FPS actual frames, 25 FPS animation */

#ifndef N_LEDS
#error "N_LEDS needs to be defined in config.h"
#endif
#ifndef N_BYTES_PER_LED
#error "N_BYTES_PER_LED  needs to be defined in config.h"
#endif

#define N_BYTES (N_LEDS*N_BYTES_PER_LED)

extern uint8_t *ws2812_fb;    /* points to current framebuffer to be drawn */
extern volatile uint8_t ws2812_wait;

void ws2812_init(void);
void ws2812_next(void);
void ws2812_next_sleep(void); /* suspend to LPM3 until the next animation fram can be drawn */
void ws2812_run(void);
void ws2812_stop(void);

#endif // WS2812_H
