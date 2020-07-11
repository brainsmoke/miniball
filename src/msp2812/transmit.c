/*
 * adapted from goodwatch' ook.c
 */

#include<stdio.h>
#include<string.h>
#include<msp430.h>

#include "radio.h"
#include "packet.h"

/* Settings were prototyped first in Python.  This is basic OOK with
   no preamble, no CRC, and 341µs symbol times.
 */
static const uint8_t ook_settings[]={
  //Change these to change the rate.
  MDMCFG4, 0x86,      // 0x80 | exponent      // Modem Configuration
  MDMCFG3, 0xd9,      // mantissa
  //These rest are consistent for all OOK emulation.
  MDMCFG2, 0x30,      // Modem Configuration, no sync
  FREND0 , 0x11,      // Front End TX Configuration
  FSCAL3 , 0xE9,      // Frequency Synthesizer Calibration
  FSCAL2 , 0x2A,      // Frequency Synthesizer Calibration
  FSCAL1 , 0x00,      // Frequency Synthesizer Calibration
  FSCAL0 , 0x1F,      // Frequency Synthesizer Calibration
  PKTCTRL0, 0x00,     // Packet automation control, fixed length without CRC.
//  PKTLEN,  LEN,       // PKTLEN    Packet length.
  0,0
};

void transmit_msg(uint8_t *msg, int len)
{
    radio_on();
    radio_writesettings(ook_settings);
  	radio_writereg(PKTLEN, len);
    radio_writepower(0x25);
    radio_setfreq(433960000);
	packet_tx(msg, len);
	radio_off();
}
