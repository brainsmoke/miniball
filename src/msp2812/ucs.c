/*! \file ucs.c
  \brief Clocking functions.
  
  This module implements a minimal driver for the Unified Clock System
  of the CC430F6137 and related devices.  In general, we try to run in
  slow mode whenever possible, jumping to fast mode only during
  diagnostics or if required (briefly) for a radio exchange.  At all
  other times, we run the CPU at 32kHz for power efficiency.
  
*/

#include <stdio.h>

#include <msp430.h>
//#include "api.h"

//! Fast mode.
void ucs_fast()
{
	// Loop until DCO stabilizes
	do
	{
		UCSCTL7 &= ~(DCOFFG|XT1LFOFFG);
		                                    // Clear DCO fault flags
		SFRIFG1 &= ~OFIFG;                  // Clear fault flags
	}
	while (SFRIFG1 & OFIFG);                // Test oscillator fault flag

	UCSCTL4 = SELA__REFOCLK | SELS__DCOCLK | SELM__DCOCLK;  // REFO for ACLK (uart), SMCLK (spi) and MCLK from DCO.
}

//! Slow mode.
void ucs_slow()
{
	UCSCTL4 = SELA__REFOCLK | SELS__VLOCLK | SELM__VLOCLK; // REFO for ACLK (uart), SMCLK (spi) and MCLK from VLO very slow CPU.
}

void ucs_init(){

  __bis_SR_register(SCG0);

  UCSCTL0 = 0x0000;
  UCSCTL1 = DCORSEL_3;
  UCSCTL2 = FLLD_0 | (FLLN0*73); /* 1 * (1*73) * REFO = 2.39MHz */
  UCSCTL3 = SELREF__REFOCLK | FLLREFDIV_0;

  __bic_SR_register(SCG0);

  ucs_slow();
  
  ucs_fast();
}
