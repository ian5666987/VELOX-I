// code by ntu ael

#ifndef C8051_SPI0_INT_H
#define C8051_SPI0_INT_H

#define   SPI_putGet   1
#define   SPI_put      0

#define SPI0int_setPriorityHigh    EIP1 |= 1

#define SPI0int_setPriorityNormal  EIP1 &= 0xfe

char SPI0int_process(char mode, void* buffer, short length);

#endif
