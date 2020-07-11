
MCU=cc430f5137

CC=msp430-gcc
OBJCOPY=msp430-objcopy
BSL=bin/cc430-bsl.py
CFLAGS=-mmcu=$(MCU) -Wall -Wextra -I. -Os -g
LDFLAGS=

LDSCRIPT=msp430.x
OBJS=dmesg.o uart.o ucs.o ws2812.o gamma_map.o rgbwaves.o ref.o lipo.o power.o radio.o packet.o transmit.o

.PHONY: flash

all: ball.elf ball.hex tx_tick.elf tx_tick.hex

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.elf: %.o $(LDSCRIPT) $(OBJS)
	$(CC) $(CFLAGS) -T $(LDSCRIPT) -o $@ $< $(OBJS) $(LDFLAGS)

flash: ball.hex
	$(BSL) -ef $< -r 38400
	cp ball.hex lastflash.hex

tflash: tx_tick.hex
	$(BSL) -ef $< -p /dev/ttyUSB3 -r 38400
	cp tx_tick.hex lastflash.hex

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -- *.hex *.elf *.o
