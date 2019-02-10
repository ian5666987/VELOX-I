// code by ntu ael

#ifndef VELOX_UART_H
#define VELOX_UART_H

void UART0_open(char* rx_buffer, short rx_size, char* tx_buffer, short tx_size, char p_uc_SSTA0);
void UART1_open(char* rx_buffer, short rx_size, char* tx_buffer, short tx_size);

char Uart_pollChars(char port, void* ptr, short length);
void Uart_advance(char port, short length);
void Uart_putChars(char port, void* ptr, short length);

#endif

