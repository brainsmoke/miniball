
Software for the miniball driver board 2.0, which is based on the
CC430F5137, and ultra-low-power msp430 chip.

This board design and firmware is based on the firmware of the Goodwatch project:
  https://goodwatch.org/
  https://github.com/travisgoodspeed/goodwatch/

This means that as per its license, I now owe Travis Goodspeed a pint of good,
hoppy pale ale.

## WS2812 Driver

The WS2812 LEDs (or in this case SK6805) are driven using a trick from
https://github.com/RoXXoR/ws2812 .  Every bit-encoding-waveform from the
WS2812 protocol is encoded using 3 data-bits in an spi master output
stream (0 -> 100, 1 -> 110).  DMA is used to make sure the spi data is sent
without hiccups at a bitrate of 2.4Mbit, resulting in a 800kHz waveform.

As with the tiny2812 implementation, animations are gamma-corrected and
use temporal dithering to still have decent colour-depth, even though the leds
are never used at full brightness.

The CPU clock runs at approx. 2.4MHz, so, 3 cycles per bit transmitted!

## TODO

- Measure LiPO battery voltage using the ADC and a voltage drop over an LED.
- Receive commands over the radio.
- Shutdown on low power.

