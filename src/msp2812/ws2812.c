
#include <msp430.h>

/* Encoding the ws2812 waveform as SPI data, like in:
 * https://github.com/RoXXoR/ws2812/blob/master/main.c
 * but with additional gamma correction & temporal dithering @ 400Hz
 */

#include "ws2812.h"
#include "gamma_map.h"

#define ENCODING_FACTOR (3) // 1 byte -> 3 bytes
#define N_DMA_BYTES (N_BYTES*ENCODING_FACTOR)

#define N_BYTES_EVEN (N_BYTES + (N_BYTES&1))              /* needed to speed up inline asm */
#define N_DMA_BYTES_BUFSZ (N_BYTES_EVEN*ENCODING_FACTOR)

static uint8_t fb0[N_BYTES_EVEN];
static uint8_t fb1[N_BYTES_EVEN];
static uint8_t rem[N_BYTES_EVEN];
static uint8_t dma_out[N_DMA_BYTES_BUFSZ];
uint8_t *ws2812_fb = fb0;
static uint8_t *fb_next = fb1;
volatile uint8_t ws2812_wait = 0;

/*
 * 30 LEDS on a 23mm half-sphere on full brightness is too bright,
 * so we only use 1/16th of the value range, [0. 15]
 *
 * An 8-bit brightness value is first gamma-corrected to a value in
 * the interval [0, 3840] (15*256).  This value, iterpreted as a 16 bit word
 * can be split in its high byte (base brightness) and low byte (brighness remainder)
 *
 * gamma_corrected = gamma_map[input_byte]
 * base, remainder = gamma_corrected // 256, gamma_corrected % 256
 *
 * Every value in the framebuffer has a 'residue' value in which the remainder
 * parts of the gamma corrected brightness accumulate every time the framebuffer
 * is displayed.  When this residue value overflows beyond 256, 1 is added to
 * the base brightness as the value sent to the LED strip for the current frame.
 *
 * outvalue, residue = base + (remainder+residue) // 256, (remainder+residue) % 256
 *
 * The gamma_map is generated in such a way that its remainder values are never
 * in the range 0-18, or 231-255, in order to prevent low fequency oscillations
 * in the output.
 *
 * the final output byte is then translated into 3 bytes of the form:
 *
 * [ 1, bit7, 0, 1, bit6, 0, 1, bit5 ] , [ 0, 1, bit4, 0, 1, bit3, 0, 1], [bit2, 0, 1, bit1, 0, 1, bit0, 0],
 *
 * which corresponds to the required waveform:
 *
 *   ____ ....      ____ ....      ____ ....      ____ ....      ____ ....      ____ ....      ____ ....      ____ ....
 *  |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
 *  |    |bit7|    |    |bit6|    |    |bit5|    |    |bit4|    |    |bit3|    |    |bit2|    |    |bit1|    |    |bit0|
 *__|    |....|____|    |....|____|    |....|____|    |....|____|    |....|____|    |....|____|    |....|____|    |....|____...
 *
 * But we can simplefy this a bit, bits 4-7 are always 0 since we only
 * send values in the interval [0, 15]. This means the first byte is
 * always the same:
 *
 * [ 1, 0, 0, 1, 0, 0, 1, 0 ] , [ 0, 1, 0, 0, 1, bit3, 0, 1], [bit2, 0, 1, bit1, 0, 1, bit0, 0],
 *
 * an optimization can be made by encoding two values 'a' and 'b' in the same loop iteration:
 *
 * [ 1, 0, 0, 1, 0, 0, 1, 0 ] , [ 0, 1, 0, 0, 1, a_bit3, 0, 1], [a_bit2, 0, 1, a_bit1, 0, 1, a_bit0, 0],
 * [ 1, 0, 0, 1, 0, 0, 1, 0 ] , [ 0, 1, 0, 0, 1, b_bit3, 0, 1], [b_bit2, 0, 1, b_bit1, 0, 1, b_bit0, 0],
 *
 * Reading whole words is more efficient than accessing RAM one byte at
 * a time, so we use two different lookup tables, 'attaching' the
 * constant first byte of the second value to the end of the first:
 *
 * [ 1, 0, 0, 1, 0, 0, 1, 0 ] , [ 0, 1, 0, 0, 1, a_bit3, 0, 1], [a_bit2, 0, 1, a_bit1, 0, 1, a_bit0, 0], [ 1, 0, 0, 1, 0, 0, 1, 0 ],
 *
 * [ 0, 1, 0, 0, 1, b_bit3, 0, 1], [b_bit2, 0, 1, b_bit1, 0, 1, b_bit0, 0],
 *
 */

const uint16_t bitmap_a[] =
{
/*
 # lookup table: [0, 15] -> bit pattern (2x uint16_t, little endian)
 #
 # pattern:
 #
 # byte 0 (msb->lsb) : [    1,    0,    0,    1,    0,    0,    1,    0 ]
 # byte 1 (msb->lsb) : [    0,    1,    0,    0,    1, bit3,    0,    1 ]
 # byte 2 (msb->lsb) : [ bit2,    0,    1, bit1,    0,    1, bit0,    0 ]
 # byte 3 (msb->lsb) : [    1,    0,    0,    1,    0,    0,    1,    0 ]
 #

def waveform3(bit):
	return 0b100+0b010*int(bit != 0)

bitpatterns = [ sum( waveform3( (1<<bit)&n != 0 ) << (bit*3) for bit in range(8) ) for n in range(16) ]
for i, p in enumerate(bitpatterns):
	b0 = (p>>16)&0xff
	b1 = (p>> 8)&0xff
	b2 = (p>> 0)&0xff
	b3 = b0
	print (hex(b1<<8 | b0)+','+hex(b3<<8 | b2)+',' ,end='')
	if i%8 == 7:
		print()
 */
0x4992,0x9224,0x4992,0x9226,0x4992,0x9234,0x4992,0x9236,0x4992,0x92a4,0x4992,0x92a6,0x4992,0x92b4,0x4992,0x92b6,
0x4d92,0x9224,0x4d92,0x9226,0x4d92,0x9234,0x4d92,0x9236,0x4d92,0x92a4,0x4d92,0x92a6,0x4d92,0x92b4,0x4d92,0x92b6,
};


const uint16_t bitmap_b[] =
{
/*
for i, p in enumerate(bitpatterns):
	b0 = (p>> 8)&0xff
	b1 = (p>> 0)&0xff
	print (hex(b1<<8 | b0)+',' ,end='')
	if i%16 == 15:
		print()

 */
0x2449,0x2649,0x3449,0x3649,0xa449,0xa649,0xb449,0xb649,0x244d,0x264d,0x344d,0x364d,0xa44d,0xa64d,0xb44d,0xb64d,
};


void ws2812_encode(void)
{

uint16_t rX=0, rP=1;
/* // Pseudo-code
 *
 * uint8_t *rRest=rem, *rBuf=fb_next;
 * uint16_t *rOut=dma_out;
 * uint16_t rCount = (N_BYTES_EVEN/2);
 *
 * // every loop translates 2 bytes framebuffer input into 6 bytes gamma corrected waveform output
 *
 * do {
 *
 * rX = *rRest++ + gamma_map[rBuf++];      // add 16 bit gamma corected value to temporal dithering remainder
 * rRest[-1] = (uint8_t)rX;                // save new remainder
 * rX = bswap16(rX);                       // byte swap value so that the low byte now contains the output color value
 *
 * rP = &bitmap_a[ (uint8_t)rX*2 ];        // calculate lookup location
 * rOut[0] = *rP++;                        // straight copy first two of three bytes of waveform
 * rOut[1] = *rP;                          // straight copy last byte of waveform + first byte of next waveform (identical for all values [0, 15])
 *
 * rX = *rRest++ + gamma_map[rBuf++];      // add 16 bit gamma corected value to temporal dithering remainder
 * rRest[-1] = (uint8_t)rX;                // save new remainder
 * rX = bswap16(rX);                       // byte swap value so that the low byte now contains the output color value
 *
 * rP = &bitmap_b[ (uint8_t)rX ];          // calculate lookup location
 * rOut[2] = *rP;                          // straight copy last byte of waveform + first byte of next waveform (identical for all values [0, 15])
 *
 * rOut = &rOut[3];
 *
 * } while (--rCount);
 */

asm (
"0:\n"
/* 2  */ "mov.b @%[rBuf]+, %[rP]\n"
/* 1  */ "add %[rP], %[rP]\n"
/* 2  */ "mov.b @%[rRest]+, %[rX]\n"
/* 3  */ "add gamma_map(%[rP]), %[rX]\n"
/* 3  */ "mov.b %[rX], -1(%[rRest])\n"
/* 1  */ "swpb %[rX]\n"
/* 1  */ "mov.b %[rX], %[rP]\n"
/* 1  */ "add %[rP], %[rP]\n"
/* 1  */ "add %[rP], %[rP]\n"
/* 1  */ "add %[rBitmapA], %[rP]\n"
/* 4  */ "mov   @%[rP]+, 0(%[rOut])\n" /* out > [ 1 0 0 1 0 0 1 0 0 1 0 0 1 A 0 1 ] */
/* 4  */ "mov   @%[rP] , 2(%[rOut])\n" /* out > [ B 0 1 D 0 1 D 0 1 0 0 1 0 0 1 0 ] */
/* 2  */ "mov.b @%[rBuf]+, %[rP]\n"
/* 1  */ "add %[rP], %[rP]\n"
/* 2  */ "mov.b @%[rRest]+, %[rX]\n"
/* 3  */ "add gamma_map(%[rP]), %[rX]\n"
/* 3  */ "mov.b %[rX], -1(%[rRest])\n"
/* 1  */ "swpb %[rX]\n"
/* 1  */ "mov.b %[rX], %[rP]\n"
/* 1  */ "add %[rP], %[rP]\n"
/* 1  */ "add %[rBitmapB], %[rP]\n"
/* 4  */ "mov   @%[rP], 4(%[rOut])\n"  /* out > [ 0 1 0 0 1 A 0 1 B 0 1 D 0 1 D 0 ] */
/* 2  */ "add #6, %[rOut]\n"
/* 1  */ "dec %[rCount]\n"
/* 2  */ "jnz 0b\n"

	::
	[rX]       "r" (rX),
	[rP]       "r" (rP),
	[rRest]    "r" (rem),
	[rBuf]     "r" (fb_next),
	[rOut]     "r" (dma_out),
	[rCount]   "r" (N_BYTES_EVEN/2),
	[rBitmapA] "r" (bitmap_a),
	[rBitmapB] "r" (bitmap_b)
	:
	"memory"
);

}

static void spi_init(void)
{
	/* initialise SPI on USCI_B0 */
	UCB0CTL1 = UCSWRST;
	UCB0CTL0 = UCMSB | UCMST | UCSYNC | UCMODE_0; /* MSB, Master, 3-pin SPI */
	UCB0BR0 = 1;
	UCB0BR1 = 0;
	UCB0CTL1 = UCSSEL_2; /* clear RST flag, select SMCLK as clock source */	
	P1SEL |= BIT3;
}

static void timer_init(void)
{
	TA1CTL = TASSEL__ACLK | TAIE;
	TA1R = 0;
	TA1CCR0 = 81; /* 32768/400 */
}

static void ws2812_transfer(void)
{
	DMA0CTL = 0;
	DMA0SA = (uint16_t)dma_out;
	DMA0DA = (uint16_t)&UCB0TXBUF;
	DMACTL0 = DMA0TSEL__USCIB0TX;
	DMA0SZ = N_DMA_BYTES;
	DMA0CTL = DMADT_0 | DMASRCINCR_3 | DMASRCBYTE | DMADSTINCR_0 | DMADSTBYTE | DMAEN;

	/* edge trigger the first transfer */
	UCB0IFG &= ~UCTXIFG;
	UCB0IFG |= UCTXIFG;
}

void ws2812_init(void)
{
	uint16_t i=0, v=0;
	for (i=0; i<N_BYTES; i++)
	{
		rem[i] = (v+=157);
		fb_next[i] = 0;
	}

	spi_init();
	timer_init();
}

void ws2812_next(void)
{
	while ( ws2812_wait ) ;
	ws2812_wait = 1;
}

void ws2812_next_sleep(void)
{
	while ( ws2812_wait ) ;
	LPM0;
}

void ws2812_run(void)
{
	TA1CTL |= MC__UP;
}

void ws2812_stop(void)
{
	TA1CTL &= ~MC__UP;
}

#define __status_register_on_exit() (*(uint16_t*)__builtin_frame_address(0))

void __attribute__ ((interrupt(TIMER1_A1_VECTOR))) ta1_timer (void)
{
	static uint16_t d_frames = 0;
	d_frames++;

	if (d_frames >= DITHER_FRAMES)
	{
		if ( (__status_register_on_exit() & LPM0_bits) == LPM0_bits )
		{
			LPM0_EXIT;
			ws2812_wait = 1;
		}
		if (ws2812_wait)
		{
			d_frames = 0;
			uint8_t *tmp = fb_next;
			fb_next = ws2812_fb;
			ws2812_fb = tmp;
			ws2812_wait = 0;
		}
	}

	ws2812_encode();

	TA1CTL &= ~TAIFG;
	ws2812_transfer();
}

