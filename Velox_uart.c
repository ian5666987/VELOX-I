// code by ntu ael

#include "C8051_UART0.h"
#include "C8051_UART1.h"

char Uart_pollChars(char port, void* buffer, short length) {
  switch (port) {
  case 0:
    return UART0_pollChars(buffer, length);
  case 1:
    return UART1_pollChars(buffer, length);
  }
  return 0;
}

void Uart_advance(char port, short length) {
  switch (port) {
  case 0:
    UART0_advance(length);
    break;
  case 1:
    UART1_advance(length);
    break;
  }
}

void Uart_putChars(char port, void* buffer, short length) {
  switch (port) {
  case 0:
    UART0_putChars(buffer, length);
    break;
  case 1:
    UART1_putChars(buffer, length);
    break;
  }
}

