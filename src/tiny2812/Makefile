
# change these to the right values

MCU = attiny85         # as known by avr-gcc
PARTNO = t85           # as known by avrdude
PROGRAMMER = usbtiny


TARGET = example_ani.hex
ELFTARGET = example_ani.elf
COMMON = gamma_map.c tiny2812.c

AVRDUDE = avrdude -v -c $(PROGRAMMER) -p$(PARTNO)

.PHONY: clean

all: $(TARGET) $(ELFTARGET)


%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

%.elf: %.c $(COMMON)
	avr-gcc -mmcu=$(MCU) -O -o $@ $^

%.o: %.S
	avr-gcc -mmcu=$(MCU) -c -o $@ $< 

%.o: %.c
	avr-gcc -O -mmcu=$(MCU) -c -o $@ $<

flash: $(TARGET)
	avrdude -v -c usbtiny -pt85 -U flash:w:$(TARGET)

fuses:
	avrdude -v -c usbtiny -pt85 -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

clean:
	-rm $(TARGET) $(ELFTARGET)
