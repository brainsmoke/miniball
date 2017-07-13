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

static int8_t c,ix;

void next_frame(void)
{
	uint8_t i,j;
	for (i=0, j=0; i<16; i++, j+=3)
	{
		if (inv_vcc & (1<<i) )
			frame[j]=255;
		else
			frame[j]=0;
		frame[j+1]=frame[j+2]=0;
	}

	for (j=48; j<90; j++)
		frame[j]=64;

	if ((c++&0x7)==0)
		ix = (ix+1)&0xf;

	frame[ix*3+1]=255;
}

void main(void)
{
	c=ix=0;
	PORTB &=~ (1<<LED_POWER_MOSFET_PIN);
	DDRB  |=  (1<<LED_POWER_MOSFET_PIN);
	_delay_loop_2(MS_TIMEOUT); /* 1 ms, seems to prevent timer from not working sometimes... brownout(?, seems unlikely) time clock init(?)  */
	init();
	run();
}

