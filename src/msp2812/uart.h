
void uart_init();
void uart_tx(uint8_t byte);
int16_t uart_rx_nonblock(void);
uint8_t uart_rx_block(void);

