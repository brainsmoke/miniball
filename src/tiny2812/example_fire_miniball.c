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

#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <util/delay_basic.h>

#define STAT_PIN (1)
#define LIPO_LOW_INV_VCC (320)
#define LIPO_HIGH_INV_VCC (272)
#define EXTERNAL_POWER_INV_VCC (253)

/*

i_low = 320
i_high = 272

low = 1./i_low
high = 1./i_high

d = (high-low)/160.

for i in range(i_high, i_low):
    v = 1./i
    print str(int( (v-low)/d))+", ",

print

*/

const uint8_t power_state[LIPO_LOW_INV_VCC-LIPO_HIGH_INV_VCC] PROGMEM = {
160,  156,  152,  148,  144,  140,  136,  133,  129,  125,  122,  118,  114,  111,  107,  104,  100,  97,  93,  90,  86,  83,  80,  76,  73,  70,  66,  63,  60,  57,  54,  50,  47,  44,  41,  38,  35,  32,  29,  26,  23,  20,  17,  14,  11,  8,  5,  2,
};

#include "tiny2812.h"

/*

import cmath

position = [
[0.689152, 0.689152, -0.223919],

[0.380954, 0.924594, 0.000000],

[0.117721, 0.924594, 0.362309],

[0.000000, 0.689152, 0.724617],
[0.072756, 0.308198, 0.948536],
[-0.308198, 0.072756, 0.948536],
[-0.616396, 0.308198, 0.724617],
[-0.425919, 0.689152, 0.586227],

[-0.308198, 0.924594, 0.223919],

[-0.689152, 0.689152, 0.223919],
[-0.879629, 0.308198, 0.362309],
[-0.997350, 0.072756, -0.000000],
[-0.879629, 0.308198, -0.362309],
[-0.689152, 0.689152, -0.223919],

[-0.308198, 0.924594, -0.223919],

[-0.425919, 0.689152, -0.586227],
[-0.616396, 0.308198, -0.724617],
[-0.308198, 0.072756, -0.948536],
[0.072756, 0.308198, -0.948536],
[0.000000, 0.689152, -0.724617],

[0.117721, 0.924594, -0.362309],

[0.425919, 0.689152, -0.586227],
[0.498675, 0.308198, -0.810146],
[0.806873, 0.072756, -0.586227],
[0.924594, 0.308198, -0.223919],

[0.924594, 0.308198, 0.223919],
[0.806873, 0.072756, 0.586227],
[0.498675, 0.308198, 0.810146],
[0.425919, 0.689152, 0.586227],
[0.689152, 0.689152, 0.223919],
]

 */

const uint8_t mapping[30] PROGMEM = {
30, 29, 44, 51, 60, 58, 56, 50, 42, 33, 40, 24, 16, 25, 18, 10, 0, 2, 4, 11, 20, 13, 6, 15, 31, 39, 55, 62, 53, 38
};

const uint8_t ring[20] PROGMEM = {
	0*3+1, 29*3+1, 28*3+1, 3*3+1, 7*3+1, 9*3+1, 13*3+1, 15*3+1, 19*3+1, 21*3+1,
	0*3+1, 29*3+1, 28*3+1, 3*3+1, 7*3+1, 9*3+1, 13*3+1, 15*3+1, 19*3+1, 21*3+1,
};

const uint8_t colortable[256*3] PROGMEM =
{
0x0,0x0,0x0, 0x1,0xb,0x0, 0x4,0x17,0x0, 0x7,0x23,0x0, 0xb,0x2f,0x0, 0x10,0x3b,0x1, 0x15,0x47,0x2, 0x1b,0x53,0x3, 0x21,0x5f,0x3, 0x28,0x6b,0x5, 0x2f,0x77,0x6, 0x36,0x83,0x7, 0x3e,0x8f,0x8, 0x46,0x9b,0xa, 0x4e,0xa7,0xc, 0x56,0xb3,0xe, 0x5f,0xbf,0xf, 0x68,0xcb,0x11, 0x72,0xd7,0x14, 0x7b,0xe3,0x16, 0x85,0xef,0x18, 0x8f,0xfb,0x1b, 0x9a,0xff,0x1e, 0xa4,0xff,0x20, 0xaf,0xff,0x23, 0xba,0xff,0x26, 0xc6,0xff,0x2a, 0xd1,0xff,0x2d, 0xdd,0xff,0x30, 0xe9,0xff,0x34, 0xf5,0xff,0x38, 0xff,0xff,0x3b, 0xff,0xff,0x3f, 0xff,0xff,0x43, 0xff,0xff,0x47, 0xff,0xff,0x4c, 0xff,0xff,0x50, 0xff,0xff,0x55, 0xff,0xff,0x59, 0xff,0xff,0x5e, 0xff,0xff,0x63, 0xff,0xff,0x68, 0xff,0xff,0x6d, 0xff,0xff,0x73, 0xff,0xff,0x78, 0xff,0xff,0x7e, 0xff,0xff,0x83, 0xff,0xff,0x89, 0xff,0xff,0x8f, 0xff,0xff,0x95, 0xff,0xff,0x9b, 0xff,0xff,0xa1, 0xff,0xff,0xa8, 0xff,0xff,0xae, 0xff,0xff,0xb5, 0xff,0xff,0xbc, 0xff,0xff,0xc3, 0xff,0xff,0xca, 0xff,0xff,0xd1, 0xff,0xff,0xd8, 0xff,0xff,0xe0, 0xff,0xff,0xe7, 0xff,0xff,0xef, 0xff,0xff,0xf7, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0

};

const uint8_t curve[256] PROGMEM =
{
/* [ int((x/64.)**1.7 / 7.7 * 64.) for x in xrange(255) ] + [ 128 ] */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 34, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 38, 39, 39, 39, 40, 40, 41, 41, 42, 42, 42, 43, 43, 44, 44, 45, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63, 64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 78, 78, 79, 79, 80, 80, 81, 81, 82, 83, 83, 84, 84, 85, 85, 86, 128
//>>> [ int((x/64.)**1.3 / 7.7 * 64.) for x in xrange(255) ] + [ 64 ]
//0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 33, 33, 34, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36, 37, 37, 37, 37, 37, 38, 38, 38, 38, 39, 39, 39, 39, 40, 40, 40, 40, 41, 41, 41, 41, 42, 42, 42, 42, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 47, 47, 47, 47, 48, 48, 48, 48, 49, 49, 49, 49, 64
};

static int8_t shown_power_state;

void halt(void)
{
	TCCR1 = 0;
	PORTB |= (1<<LED_POWER_MOSFET_PIN);
	_delay_loop_2(MS_TIMEOUT); /* 1 ms */
	sleep_mode();
}


void charging(void)
{
	static int8_t start = 0, size = 0, c=0;
	uint8_t i;

	c+=64;
	if (c==0)
		size++;
	if (size == 20)
	{
		size = 0;
		start++;
	}
	if (start == 10)
		start = 0;

	for (i=0; i<3*30; i++)
		frame[i] = 0;

	for (i=0; i<size && i<10; i++)
		frame[pgm_read_byte_near(ring+start+i)] = 255;

	for (i=10; i<size; i++)
		frame[pgm_read_byte_near(ring+start+i-10)] = 0;

	if (size < 10)
		frame[pgm_read_byte_near(ring+start+size)] = c;
	else
		frame[pgm_read_byte_near(ring+start+size-10)] = 255-c;
}

void show_power_state(void)
{
	uint8_t i;
	uint8_t p_state;

	uint16_t ivcc = inv_vcc;

	if (ivcc > LIPO_LOW_INV_VCC)
		p_state = 0;
	else if (ivcc <= LIPO_HIGH_INV_VCC)
		p_state = 160;
	else
		p_state = pgm_read_byte_near(power_state+ivcc-LIPO_HIGH_INV_VCC);

	for (i=0; i<3*30; i++)
		frame[i] = 0;

	for (i=0; i<3; i++)
	{
		if (p_state < 16)
		{
			frame[pgm_read_byte_near(ring+i)] = p_state*16;
			return;
		}
		frame[pgm_read_byte_near(ring+i)] = 255;
		p_state -= 16;
	}

	for (i=3; i<7; i++)
	{
		if (p_state < 16)
		{
			frame[pgm_read_byte_near(ring+i)-1] = p_state*10;
			frame[pgm_read_byte_near(ring+i)] = p_state*10;
			return;
		}
		frame[pgm_read_byte_near(ring+i)-1] = 160;
		frame[pgm_read_byte_near(ring+i)] = 160;
		p_state -= 16;
	}


	for (i=7; i<10; i++)
	{
		if (p_state < 16)
		{
			frame[pgm_read_byte_near(ring+i)-1] = p_state*16;
			return;
		}
		frame[pgm_read_byte_near(ring+i)-1] = 255;
		p_state -= 16;
	}
}


/* https://www.avrfreaks.net/forum/tiny-fast-prng */
uint8_t prng(void)
{
	static uint8_t s=0xaa, a=0;
	s^=s<<3;
	s^=s>>5;
	s^=a++>>2;
	return s;
}

uint8_t automata[64+8];
uint8_t iter = 0;


void next_frame(void)
{
	if (inv_vcc > LIPO_LOW_INV_VCC)
	{
		halt();
		shown_power_state = 0;
		return;
	}

	if ( (inv_vcc <= EXTERNAL_POWER_INV_VCC) && !( PINB & (1<<STAT_PIN) ) )
	{
		charging();
		shown_power_state = 0;
		return;
	}

	if ( (inv_vcc > EXTERNAL_POWER_INV_VCC) && shown_power_state < 30 )
	{
		show_power_state();
		shown_power_state++;
		return;
	}

	uint8_t i,j , bit, c, r, y;
	uint16_t l;

	if ( (iter & 3) == 0)
	{
		r = prng();
		for (i=0; i<8; i++,r>>=1)
			if (r & 1)
				automata[64+i] = 44;
			else
				automata[64+i] = 0;
	}
	iter++;

	for (y=0; y<64; y+=8)
	{
		l = automata[y+8] + automata[y+9];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y] += -(automata[y]>>2)+c;

		l += automata[y+10];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y+1] += -(automata[y+1]>>2)+c;

		l -= automata[y+8];
		l += automata[y+11];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y+2] += -(automata[y+2]>>2)+c;

		l -= automata[y+9];
		l += automata[y+12];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y+3] += -(automata[y+3]>>2)+c;

		l -= automata[y+10];
		l += automata[y+13];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y+4] += -(automata[y+4]>>2)+c;

		l -= automata[y+11];
		l += automata[y+14];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y+5] += -(automata[y+5]>>2)+c;

		l -= automata[y+12];
		l += automata[y+15];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y+6] += -(automata[y+6]>>2)+c;
		l -= automata[y+13];
		c = 128;
		if (l < 255)
			c = pgm_read_byte_near(curve+l);
		automata[y+7] += -(automata[y+7]>>2)+c;
	}

	j=0;
	for (i=0; i<30; i++)
	{ 
		uint16_t color_ix = automata[pgm_read_byte_near(mapping+i)];
        color_ix = color_ix+color_ix+color_ix;
		frame[j++] = pgm_read_byte_near(colortable+color_ix);
		color_ix++;
		frame[j++] = pgm_read_byte_near(colortable+color_ix);
		color_ix++;
		frame[j++] = pgm_read_byte_near(colortable+color_ix);
		color_ix++;
	}
}

void main(void)
{
uint8_t i;
for(i=0; i<64; i++)
	automata[i] = 0;

	shown_power_state = 0;
	PORTB &=~ (1<<LED_POWER_MOSFET_PIN);
	DDRB  |=  (1<<LED_POWER_MOSFET_PIN);
	PORTB |=  (1<<STAT_PIN); /* internal pull-up */
	_delay_loop_2(MS_TIMEOUT); /* 1 ms, seems to prevent timer from not working sometimes... brownout(?, seems unlikely) time clock init(?)  */
	init();
	run();
}

