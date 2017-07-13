#ifndef TINY2812_H
#define TINY2812_H

#include <stdint.h>

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#if F_CPU == 8000000
#define MS_TIMEOUT (2000)
#elif F_CPU == 16000000
#define MS_TIMEOUT (4000)
#endif

/* measure VCC by default */
#ifndef NO_MEASURE_VCC 
#define MEASURE_VCC
#endif

#define OUTPORT PORTB
#define OUTPIN  4
#define LED_POWER_MOSFET_PIN 3

#define N_LEDS (30)
#define N_BYTES (3*N_LEDS)

#define TEMP_DIFF_FRAMES 16

#ifdef MEASURE_VCC
extern volatile uint16_t inv_vcc; /* uncalibrated inverted bandgap voltage measurement inv_vcc = 1024*bandgap/vcc */
#endif

extern uint8_t *frame; /* buffer to paint on */

/* Initializes buffers and timer */
void init(void);

/* Endless loop, calling next_frame() & suspending to save energy */
void run(void);

/* to be implemented by animation creator */
void next_frame(void);

#endif

