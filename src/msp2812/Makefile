
MCU=cc430f5137

CC=msp430-gcc
OBJCOPY=msp430-objcopy
BSL=bin/cc430-bsl.py
CFLAGS=-mmcu=$(MCU) -Wall -Wextra -I. -Os -g
LDFLAGS=

LDSCRIPT=msp430.x
OBJS=main.o dmesg.o uart.o monitor.o ucs.o ws2812.o gamma_map.o rgbwaves.o ref.o lipo.o

.PHONY: flash

all: ball.elf ball.hex

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

ball.elf: $(LDSCRIPT) $(OBJS)
	$(CC) $(CFLAGS) -T $(LDSCRIPT) -o $@ $(OBJS) $(LDFLAGS)

flash: ball.hex
	$(BSL) -ef $< -r 38400

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -- *.hex *.elf *.o
