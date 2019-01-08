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
//#include "rng.h"
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


//! Power On Self Test
int post(){
/*
  if (has_radio && RF1AIFERR & 2){
  }else if(has_radio && RF1AIFERR & 4){
  }else if(has_radio && RF1AIFERR & 8){
  }else{
    return 0;
  }

  printf("POST failure.\n");
  //We had a failure, indicated above.
  return 1;
*/
return 0;
}

//! Main method.
int main(void) {

	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	dmesg_init();

  //printf("Initializing RNG ");
  //srand(true_rand()); // we do this as early as possible, because it messes with clocks

  /*printf("rtc ");
  lcd_string("RTC INIT");
  rtc_init();
*/
//	P2OUT &= ~BIT2;
//	P2DIR |= BIT2;

	ws2812_init();
	power_up_leds();


	ucs_init();
	__delay_cycles(30000);

	printf("uart\n");
	uart_init();
	ref_init();
	lipo_init();

	//printf("radio\n");
	//radio_init();
  
	 printf("Beginning POST.\n");
	// Run the POST until it passes.
	while( post() );

  // Setup and enable WDT 250ms, ACLK, interval timer
//  WDTCTL = WDT_ADLY_250;
//  SFRIE1 |= WDTIE;

	printf("Booted.\n");
	__bis_SR_register(GIE);

	rgbwaves_init();

	ws2812_run();

	int f=0, vOK=1, vnew;
	for (;;)
	{
		f++;
		if ((f & 0x3f) == 0)
		{
			vnew = lipo_voltage_ok_check();
			if (vnew != vOK)
			{
				vOK = vnew;
				if (vOK)
					power_up_leds();
				else
				{
					frame_clear(ws2812_fb);
					ws2812_next_sleep();
					frame_clear(ws2812_fb);
					power_down_leds();
				}
			}
		}

		if (vOK)
			rgbwaves_frame(ws2812_fb);

		ws2812_next_sleep();
	}

	//__bis_SR_register(LPM3_bits + GIE);        // Enter LPM3
	//__bis_SR_register(LPM2_bits + GIE);        // Enter LPM2
	//__bis_SR_register(LPM0_bits + GIE);	     // Enter LPM0 w/interrupt

	while(1){
		printf("q = %d\n", q);
//    printf("main while().\n");
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
