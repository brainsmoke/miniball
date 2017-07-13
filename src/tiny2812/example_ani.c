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
#include <util/delay_basic.h>

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

rot_pos = []

rot = cmath.rect(1, cmath.pi/4)

for x, y, z in position:
	q = complex(x, y)*rot
	x, y = q.real, q.imag
	q = complex(y, z)*rot
	y, z = q.real, q.imag
	rot_pos += [[x, y, z]]

xaxis = [ int(256*(cmath.phase(complex(y,z))/cmath.pi/2+.5)) for x, y, z in rot_pos ]
yaxis = [ int(256*(cmath.phase(complex(z,x))/cmath.pi/2+.5)) for x, y, z in rot_pos ]
zaxis = [ int(256*(cmath.phase(complex(x,y))/cmath.pi/2+.5)) for x, y, z in rot_pos ]

print "const uint8_t xaxis[30] PROGMEM = {"
print ", ".join(str(x) for x in xaxis)
print "};"

print "const uint8_t yaxis[30] PROGMEM = {"
print ", ".join(str(x) for x in yaxis)
print "};"

print "const uint8_t zaxis[30] PROGMEM = {"
print ", ".join(str(x) for x in zaxis)
print "};"

 */

const uint8_t xaxis[30] PROGMEM = {
150, 160, 178, 199, 212, 231, 235, 211, 179, 224, 2, 32, 61, 96, 140, 108, 84, 88, 107, 120, 141, 133, 120, 129, 149, 170, 190, 199, 186, 169
};
const uint8_t yaxis[30] PROGMEM = {
128, 106, 102, 106, 120, 109, 84, 88, 84, 70, 62, 41, 40, 57, 70, 49, 31, 13, 13, 50, 81, 90, 228, 190, 158, 148, 150, 133, 120, 128
};
const uint8_t zaxis[30] PROGMEM = {
192, 213, 238, 13, 50, 50, 32, 14, 249, 6, 23, 22, 1, 249, 235, 231, 235, 210, 199, 213, 217, 199, 186, 169, 171, 161, 129, 91, 229, 192
};

/*

from math import cos, pi

 */

const uint8_t wave[256] PROGMEM =
{
//print ', '.join(str(int(-cos(x*pi/64)*127.5+127.5)) for x in range(128) + [0]*128)
0, 0, 0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 21, 25, 28, 33, 37, 41, 46, 51, 56, 61, 67, 72, 78, 84, 90, 96, 102, 108, 115, 121, 127, 133, 139, 146, 152, 158, 164, 170, 176, 182, 187, 193, 198, 203, 208, 213, 217, 221, 226, 229, 233, 236, 239, 242, 245, 247, 249, 251, 252, 253, 254, 254, 255, 254, 254, 253, 252, 251, 249, 247, 245, 242, 239, 236, 233, 229, 226, 221, 217, 213, 208, 203, 198, 193, 187, 182, 176, 170, 164, 158, 152, 146, 139, 133, 127, 121, 115, 108, 102, 96, 90, 84, 78, 72, 67, 61, 56, 51, 46, 41, 37, 33, 28, 25, 21, 18, 15, 12, 9, 7, 5, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

//print ', '.join(str(int(-cos(x*pi/32)*127.5+127.5)) for x in range(64) + [0]*192)
//0, 0, 2, 5, 9, 15, 21, 28, 37, 46, 56, 67, 78, 90, 102, 115, 127, 139, 152, 164, 176, 187, 198, 208, 217, 226, 233, 239, 245, 249, 252, 254, 255, 254, 252, 249, 245, 239, 233, 226, 217, 208, 198, 187, 176, 164, 152, 139, 127, 115, 102, 90, 78, 67, 56, 46, 37, 28, 21, 15, 9, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

//print ', '.join(str(int(-cos(x*pi/128)*127.5+127.5)) for x in range(256))
//0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 26, 28, 30, 33, 35, 37, 39, 41, 44, 46, 49, 51, 54, 56, 59, 61, 64, 67, 70, 72, 75, 78, 81, 84, 87, 90, 93, 96, 99, 102, 105, 108, 111, 115, 118, 121, 124, 127, 130, 133, 136, 139, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 176, 179, 182, 184, 187, 190, 193, 195, 198, 200, 203, 205, 208, 210, 213, 215, 217, 219, 221, 224, 226, 228, 229, 231, 233, 235, 236, 238, 239, 241, 242, 244, 245, 246, 247, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 254, 254, 254, 255, 254, 254, 254, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 246, 245, 244, 242, 241, 239, 238, 236, 235, 233, 231, 229, 228, 226, 224, 221, 219, 217, 215, 213, 210, 208, 205, 203, 200, 198, 195, 193, 190, 187, 184, 182, 179, 176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 139, 136, 133, 130, 127, 124, 121, 118, 115, 111, 108, 105, 102, 99, 96, 93, 90, 87, 84, 81, 78, 75, 72, 70, 67, 64, 61, 59, 56, 54, 51, 49, 46, 44, 41, 39, 37, 35, 33, 30, 28, 26, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10, 9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0
};


static int16_t r,g,b;

void next_frame(void)
{
	uint8_t i, j, c;

	j=0;
	r+=11; g+=13; b+=15;
	for (i=0; i<30; i++)
	{ 
		c = (g>>2)+pgm_read_byte_near(xaxis+i);
		frame[j++] = pgm_read_byte_near(wave+c);
		c = (r>>2)+pgm_read_byte_near(yaxis+i);
		frame[j++] = pgm_read_byte_near(wave+c);
		c = (b>>2)+pgm_read_byte_near(zaxis+i);
		frame[j++] = pgm_read_byte_near(wave+c);
	}
}

void main(void)
{
	r=0,g=100,b=200;
	PORTB &=~ (1<<LED_POWER_MOSFET_PIN);
	DDRB  |=  (1<<LED_POWER_MOSFET_PIN);
	_delay_loop_2(MS_TIMEOUT); /* 1 ms, seems to prevent timer from not working sometimes... brownout(?, seems unlikely) time clock init(?)  */
	init();
	run();
}

