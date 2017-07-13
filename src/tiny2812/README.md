
tiny2812
========

Drive a small number of WS2811/WS2812/WS2812b/SK6812 LEDs with
on-the-fly temporal dithering and gamma correction using an AVR
(like the attiny85) running at 8MHz or 16MHz, perfect for tiny
wearables.

A timer triggered interrupt handler bitbangs frames while keeping
track of temporal dithering remainders.  The time in between
interrupts can be spent generating new frames.

More than *9 x number_of_leds* bytes of RAM are needed due to
double buffers and temporal dithering bookkeeping.

The code (without the animation generation code) takes up about
1K of flash memory including the gamma correction table.

API
===

```C

extern uint8_t *frame; /* buffer to paint on */

/* Initializes buffers and timer */
void init(void);

/* Endless loop, calling next_frame() & suspending to save energy */
void run(void);

/* to be implemented by animation creator */
void next_frame(void);

```

Gamma\_map.py
=============

Generate the gamma correction map used by the bitbang code.

Example:

```sh
python gamma_map.py 0x3f00 0x18 2.5
```

This generates a gamma map with a max brightness of 0x3f (63),
no temporal dithering with a period < 0x18/0x100 to prevent flickering,
and a gamma of 2.5.

MEASURE\_VCC / inv\_vcc
=======================

The code also periodically measures the bandgap voltage compared to the
supply voltage.  This may be used to shut down the power supply to the
LEDs when a LiPo/LiIon battery is drained.  However, this bandgap voltage
is not calibrated out-of-factory and can vary a lot between individual
chips.

```C
#ifdef MEASURE_VCC
extern volatile uint16_t inv_vcc; /* uncalibrated inverted bandgap voltage measurement inv_vcc = 1024*bandgap/vcc */
#endif
```

