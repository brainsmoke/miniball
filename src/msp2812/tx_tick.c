/*
 * Severly stripped down version of the goodwatch main code
 */


/*! \file main.c

  \brief Main module.  This version just initializes the LCD and then
   drops to a low power mode, letting the WDT do the work on a slow
   interval.
*/

#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#include "api.h"
#include "dmesg.h"
#include "uart.h"
#include "ucs.h"
#include "ref.h"
#include "lipo.h"
#include "dmesg.h"
#include "radio.h"
#include "packet.h"
#include "transmit.h"
#include "ws2812.h"
#include "rgbwaves.h"

static int q=0;

void power_up_leds(void)
{
	__delay_cycles(3000);
	P1OUT &= ~BIT2;
	P1DIR |=  BIT2;
	__delay_cycles(3000);
}

void power_down_leds(void)
{
	P1OUT |=  BIT2;
}

void frame_clear(uint8_t *fb)
{
	uint16_t i=0;
	for (i=0; i<N_BYTES; i++)
		fb[i] = 0;
}

//! Provide an incoming packet.
void app_packetrx(uint8_t *packet, int len)
{
	char *p = "Incoming Packet\n";
	for ( ; *p ; p++ )
		uart_tx(*p);

	int i;
	for (i=0 ; i<len ; i++ )
	{
		uart_tx(' ');
		uart_tx("0123456789ABCDEF"[(packet[i]>>4)&15] );
		uart_tx("0123456789ABCDEF"[packet[i]&15] );
	}
	uart_tx('\n');
}

//! Callback after a packet has been sent.
void app_packettx()
{
	char *p = "Packet sent\n";
	for ( ; *p ; p++ )
		uart_tx(*p);
}


//! Power On Self Test
int post(){

  if (has_radio && RF1AIFERR & 2){
  }else if(has_radio && RF1AIFERR & 4){
  }else if(has_radio && RF1AIFERR & 8){
  }else{
    return 0;
  }

  uart_tx('.');
  printf("POST failure.\n");
  //We had a failure, indicated above.
  return 1;
}

//! Main method.
int main(void) {

	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	dmesg_init();

	ucs_init();
	__delay_cycles(30000);

	printf("uart\n");
	uart_init();
	ref_init();
	lipo_init();

	printf("radio\n");
	radio_init();
	packet_init();
  
	printf("Beginning POST.\n");
	// Run the POST until it passes.
	while( post() );

	printf("Booted.\n");
	__bis_SR_register(GIE);

	for(;;)
	{
		uint8_t c = uart_rx_block();
		if (c == '.')
		{
			uint16_t i;
			for (i=0 ; i<dmesg_index; i++)
			{
				uint8_t c = dmesg_buffer[i];
				if (c == '\n')
					uart_tx('\r');
				uart_tx(c);
			}
			dmesg_clear();
		}
		transmit_msg(TICK, TICK_LEN);
	}

}

//! Watchdog Timer interrupt service routine, calls back to handler functions.
void __attribute__ ((interrupt(WDT_VECTOR))) watchdog_timer (void) {
//__bic_SR_register_on_exit();
//__bis_SR_register_on_exit();
q++;
    uart_tx('X');
//  static int latch=0;

  /* When the UART is in use, we don't want to hog interrupt time, so
     we will silently return.
  */
//  if(uartactive)
//    return;

    /* Similarly, we'll reboot if the SET/PRGM button has been held for 10
       seconds (40 polls).  We'll draw a countdown if getting close, so
       there's no ambiguity as to whether the chip reset.
       
       The other features of this button are handled within each application's
       draw function.
    */
//    if(latch++>40)
//      PMMCTL0 = PMMPW | PMMSWPOR;

}
