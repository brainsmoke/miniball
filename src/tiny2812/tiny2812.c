/*
 * Copyright (c) 2017 Erik Bosman <erik@minemu.org>
 *
 * Permission  is  hereby  granted,  free  of  charge,  to  any  person
 * obtaining  a copy  of  this  software  and  associated documentation
 * files (the "Software"),  to deal in the Software without restriction,
 * including  without  limitation  the  rights  to  use,  copy,  modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the
 * Software,  and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The  above  copyright  notice  and this  permission  notice  shall be
 * included  in  all  copies  or  substantial portions  of the Software.
 *
 * THE SOFTWARE  IS  PROVIDED  "AS IS", WITHOUT WARRANTY  OF ANY KIND,
 * EXPRESS OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR  PURPOSE  AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN
 * CONNECTION  WITH THE SOFTWARE  OR THE USE  OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * (http://opensource.org/licenses/mit-license.html)
 */

#include "tiny2812.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay_basic.h>
#include <stdlib.h>

#include <avr/pgmspace.h>

#include "gamma_map.h"

uint8_t frame_buf1[N_BYTES+1];
uint8_t frame_buf2[N_BYTES+1];
uint8_t td_rem[N_BYTES+1];
uint8_t *out_frame;
uint8_t *frame;
volatile uint8_t count;
volatile uint8_t done;

#ifdef MEASURE_VCC
volatile uint8_t vcc_test_count;
volatile uint16_t inv_vcc;
#endif


#define NOP "nop \n"

/* Dirty trick learned from Adafruit's NeoPixel library:
 * Save memory by using a slower NOP, keep the loop
 * small enough for the relative jump at the end.
 *
 * https://github.com/adafruit/Adafruit_NeoPixel/blob/master/Adafruit_NeoPixel.cpp#L204
 */
#define NOP2 "rjmp .+0 \n"
#define NOP3 "rjmp .+0 \n nop \n"

/*
 *  at  8MHz: high_cycles = 1, data_cycles = 3, low_cycles = 2
 *
 *  0+1 pin HIGH                   0/8e6 s   0
 *  1+1 high_cycles
 *  2+1 conditional skip
 *  3+1 (conditional) pin LOW      3/8e6 s   0.375us
 *  4+3 data_cycles
 *  7+1 pin LOW                    7/8e6 s   0.875us
 *  8+2 low_cycles
 * 10   (next bit)                10/8e6 s   1.25us
 *
 *  at 16MHz: high_cycles = 4, data_cycles = 7, low_cycles = 5
 *
 *  0+1 pin HIGH                   0/16e6 s  0
 *  1+4 high_cycles
 *  5+1 conditional skip
 *  6+1 (conditional) pin LOW      6/16e6 s  0.375us
 *  7+7 data_cycles
 * 14+1 pin LOW                   14/16e6 s  0.875us
 * 15+5 low_cycles
 * 20   (next bit)                20/16e6 s  1.25us
 *
 */
#define TRANSMIT_BIT(bit, high_cycles, data_cycles, low_cycles) \
	"out %[out], %[hi]\n" \
	high_cycles "\n" \
	"sbrs %[odata], " #bit "\n" \
	"out %[out], %[lo]\n" \
	data_cycles "\n" \
	"out %[out], %[lo]\n" \
	low_cycles "\n"

void display(uint8_t *buf, uint8_t size)
{
	uint8_t data, odata, rem;
	uint8_t hi=OUTPORT|(1<<OUTPIN);
	uint8_t lo=OUTPORT&~(1<<OUTPIN);
	uint8_t *remp = td_rem;
	const uint8_t *g = gamma_map;

	asm volatile(
		"ld %[data], %a[buf]+ \n"
		"add %A[g], %[data] \n"
		"adc %B[g], __zero_reg__ \n"
		"lpm %[data], %a[g] \n"
		"inc %B[g] \n"
		"ld %[rem], %a[remp] \n"
		"add %[rem], %[data] \n"
		"lpm %[odata], %a[g] \n"
		"st %a[remp]+, %[rem] \n"
		"adc %[odata], __zero_reg__ \n" /* zero flag is cleared */

#if F_CPU == 8000000

		"display_loop_start:\n"

		/* Pseudo code:
		 *
		 * do
		 * {
		 *     uint8_t *g = gamma_map;
		 *     uint8_t data = *buf++;
		 *     g += data;                        ( *g points at low byte of 16 bit gamma map entry of index [data] )
		 *     uint8_t rem = *remp;
         *     data = *g;
         *     g += 0x100;                       ( *g points at high byte of 16 bit gamma map entry of index [data] )
		 *     carry = overflows( rem += data );
		 *     data = *g + (carry ? 1 : 0);
		 *     *remp++ = rem;
		 *     odata = data;
		 * } while (size != 0);
		 */

 		/*  per transmitted bit we can spend: 1 cycle + 3 cycles + 2 cycles */
		TRANSMIT_BIT(7,
			"ldi %A[g], lo8(gamma_map)",
			"ldi %B[g], hi8(gamma_map) \n" NOP2,
			NOP2
		)
		TRANSMIT_BIT(6,
			"nop",
			"ld %[data], %a[buf]+ \n\t add %A[g], %[data]",
			"ld %[rem], %a[remp]"
		)
		TRANSMIT_BIT(5,
			"adc %B[g], __zero_reg__",
			"lpm %[data], %a[g]",
			NOP2
		)
		TRANSMIT_BIT(4,
			"nop",
			NOP3,
			NOP2
		)
		TRANSMIT_BIT(3,
			"inc %B[g]",
			"add %[rem], %[data] \n " NOP2,
			NOP2
		)
		TRANSMIT_BIT(2,
			"nop",
			"lpm %[data], %a[g]",
			NOP2
		)
		TRANSMIT_BIT(1,
			"nop",
			"st %a[remp]+, %[rem] \n adc %[data], __zero_reg__",
			NOP2
		)
		TRANSMIT_BIT(0,
			"dec %[size]",
			"mov %[odata], %[data] \n" NOP2,
			"brne display_loop_start"
		)
#elif F_CPU == 16000000

 		/*  per transmitted bit we can spend: 4 cycle + 7 cycles + 5 cycles */
		"ldi %[rem], 13 \n"                 /* [p]re-use remainder register as bit counter */
		"display_loop_pre:\n"
		"nop\n"	                            /* 3 +  1        <  */
		"display_loop_start:\n"
		TRANSMIT_BIT(7,

			"ldi %A[g], lo8(gamma_map) \n"  /* 0 +  1 */
			"ldi %B[g], hi8(gamma_map) \n"  /* 1 +  1 */
			"dec %[rem] \n"                 /* 2 +  1 */
			NOP                             /* 3 +  1 */
			,
			"brne 1f \n"                    /* 0 + 1|2 */
			"ld %[data], %a[buf]+ \n"       /* 1 +  2  */
			"rjmp 2f \n"                    /* 3 +  2  */
			"1: \n"
			NOP3 "\n"                       /* 2 +  3  */
			"2:"
			"lsl %[odata] \n"               /* 6 +  1 */  /* shift bitbang bits */
			"add %A[g], %[data] \n"         /* 5 +  1 */
			,
			NOP                             /* 0 +  1 */
			"dec %[rem] \n"                 /* 1 +  1 */
			"brpl display_loop_pre\n"       /* 2 + 1|2       ^ */
			"ld %[rem], %a[remp] \n"        /* 3 +  2 */) /* from here on rem is used as remainder */
		TRANSMIT_BIT(7,
			"adc %B[g], __zero_reg__\n"     /* 1 */
			"lpm %[data], %a[g] \n"         /* 3 */
			,
			"inc %B[g] \n"                  /* 1 */
			"add %[rem], %[data] \n"        /* 1 */
			"st %a[remp]+, %[rem] \n"       /* 2 */
			"lpm %[odata], %a[g] \n"        /* 3 */ /* destroying odata, not needed anymore */
			,
			"adc %[odata], __zero_reg__ \n" /* 1 */ /* add the temp dith overflow to output data */
			"dec %[size] \n"                /* 1 */
			"ldi %[rem], 13 \n"             /* 1 */
			"brne display_loop_start\n"     /* 2 */ )
	
#else

#error "unsupported bitrate"

#endif

		:
		[g]       "+z" (g),
		[buf]     "+e" (buf),
		[remp]    "+e" (remp),
		[size]    "+r" (size),
		[rem]     "+r" (rem),
		[data]    "+r" (data),
		[odata]   "+r" (odata)
		: 
		[out]     "I"  (_SFR_IO_ADDR(OUTPORT)),
		[hi]      "r" (hi),
		[lo]      "r" (lo)
		);

}

#ifdef MEASURE_VCC
static void vcc_test_enable(void)
{
	ADMUX = (1<<MUX2)|(1<<MUX3);
	ADCSRA = (1<<ADEN) | /* 64x prescaler */ (1<<ADPS2) | (1<<ADPS1) ;
}

static void vcc_test_disable(void)
{
	ADCSRA = 0;
}

static uint16_t adc_read(void)
{
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	return ADC;
}
#endif

ISR(TIMER1_COMPA_vect)
{
	display(out_frame, N_BYTES);
	count += 1;

#ifdef MEASURE_VCC
	/* measure VCC every 256 animation frames */
	if ( (vcc_test_count == 0) && (count == TEMP_DIFF_FRAMES-1) )
		vcc_test_enable();
#endif

	if ( (done) && (count >= TEMP_DIFF_FRAMES) )
	{
		uint8_t *tmp = out_frame;
		out_frame = frame;
		frame = tmp;
		count = done = 0;

#ifdef MEASURE_VCC
		if (vcc_test_count == 0)
		{
			inv_vcc = adc_read();
			vcc_test_disable();
		}
		vcc_test_count += 1;
#endif
	}
}

void init(void)
{
	int i=0,j=0;
	for (i=0; i<N_BYTES; i++)
	{
/* BSS
		frame_buf1[i] = 0;
*/
		td_rem[i] = (j+=153);
	}

	out_frame = frame_buf1;
	frame = frame_buf2;

/* BSS
	done = 0;
	count = 0;
*/

	PORTB &=~ (1<<OUTPIN);
	DDRB   =  (1<<OUTPIN);

#ifdef MEASURE_VCC
	vcc_test_count = 1;
	vcc_test_enable();
	_delay_loop_2(MS_TIMEOUT);
	inv_vcc = adc_read();
	vcc_test_disable();
#endif

	cli();
	TCCR1 = 0; /* stop timer */
	OCR1C = (19*F_CPU)/8000000UL;                                      /* 8e6/1024/19 ~= 411 fps */
	TIFR = (1 << OCF1A);
	TIMSK = (1 << OCIE1A);                           /* interrupt when TCNT1 == 0 */
	sei();
	_delay_loop_2(MS_TIMEOUT); /* 1 ms, seems to prevent timer from not working sometimes... brownout(?, seems unlikely) time clock init(?)  */
}

void run(void)
{
	TCCR1 = (1 << CTC1) | (1 << CS13) | (1 << CS11) | (1 << CS10); /* 8e6/1024 */

	/* main loop */

	for(;;)
	{
		next_frame();
		done = 1;
		while (done)
			sleep_mode();
	}
}

