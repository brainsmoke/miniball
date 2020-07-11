/*! \file packet.h
  \brief Packet handling library.
*/

#include<stdint.h>

//! Length of the packet buffer.
#define PACKETLEN 128

//! Receive packet buffer.
extern uint8_t rxbuffer[];
//! Transmit packet buffer.
extern uint8_t txbuffer[];

void packet_init(void);

//! Switch to receiving packets.
void packet_rxon();

//! Stop receiving packets.
void packet_rxoff();

//! Transmit a packet.
void packet_tx(uint8_t *buffer, uint8_t length);


//! Provide an incoming packet.
void app_packetrx(uint8_t *packet, int len);
//! Callback after a packet has been sent.
void app_packettx();

