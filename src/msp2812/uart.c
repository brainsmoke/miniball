/*! \file uart.c
  \brief UART Driver for the GoodWatch
  
  
  The goal here is to use P1.5 and P1.6 as a UART, just as the BSL
  does.

  Because we *also* use these pins for the MODE and SET buttons, we
  need to be a bit tricky about their initialization, and the UART may
  not be used at the same time as those watch buttons.  The general
  idea is that uartactive will be set to 1 when the UART receives its
  first commands.
  
  Similarly, we can't handle the UART at our normal rate of 32kHz, so
  the first command must be sent as individual bytes to raise the CPU
  speed.

*/

#include<stdint.h>
#include<stdio.h>
#include<msp430.h>

#include "uart.h"
#include "monitor.h"

void uart_init(){
  printf("Initializing UART ");

  PMAPPWD = 0x02D52;
  P1MAP5 = PM_UCA0RXD; // Map UCA0RXD output to P1.5 
  P1MAP6 = PM_UCA0TXD; // Map UCA0TXD output to P1.6 
  PMAPPWD = 0;
  
  P1DIR |= BIT6;         // Set P1.6 as TX output
  P1SEL |= BIT5 + BIT6;  // Select P1.5 & P1.6 to UART function

  UCA0CTL1 |= UCSWRST;                 // **Put state machine in reset**
  UCA0CTL1 |= UCSSEL_1;                // ACLK
  UCA0BR0 = 3;                         // 32kHz 9600 (see User's Guide)
  UCA0BR1 = 0;                         // 32kHz 9600
  UCA0MCTL = UCBRS_3 + UCBRF_0;        // Modln UCBRSx=0, UCBRFx=0,

  UCA0CTL1 &= ~UCSWRST;                // **Initialize USCI state machine**
  //UCA0IE |= UCRXIE;                    // Enable USCI_A0 RX interrupt
  //UCA0IE |= UCTXIE;                    // Enable USCI_A0 TX interrupt

}

void uart_tx(uint8_t byte)
{
	while (!(UCA0IFG&UCTXIFG));
	UCA0TXBUF = byte;
}

int16_t uart_rx_nonblock(void)
{
	if (!(UCA0IFG&UCRXIFG))
		return -1;
	return (uint8_t)UCA0RXBUF;
}

uint8_t uart_rx_block(void)
{
	while (!(UCA0IFG&UCRXIFG));
	return (uint8_t)UCA0RXBUF;
}

