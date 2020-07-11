
TARGETS=avrrc2812.elf avrrc2812.hex

all: $(TARGETS)

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

%.elf: %.o
	avr-ld -m avrtiny -o $@ $^

%.o: %.S
	avr-gcc -mmcu=attiny4 -c -o $@ $^

clean:
	-rm -- $(TARGETS)
