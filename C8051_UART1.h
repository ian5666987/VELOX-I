// code by ntu ael

#ifndef C8051_UART1_H
#define C8051_UART1_H

void UART1_open(char* rx_buffer, short rx_size, char* tx_buffer, short tx_size);
short UART1_getRxAvailable();
char UART1_pollChars(void* msg, short length);
void UART1_advance(short length);
void UART1_putChars(void* ptr, short len);
void UART1_close();
char UART1_getTxErrorCount();

#endif

