// code by ntu ael

#ifndef C8051_UART0_H
#define C8051_UART0_H

// the polling of this variable is optional
void UART0_open(char* rx_buffer, short rx_size, char* tx_buffer, short tx_size, char p_uc_SSTA0);
short UART0_getRxAvailable();
char UART0_pollChars(void* ptr, short length);
void UART0_advance(short length);
void UART0_putChars(void* ptr, short length);
void UART0_close();
char UART0_getRxErrorCount();
char UART0_getTxErrorCount();

#endif
