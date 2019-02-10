// code by ntu ael

#ifndef C8051_SPI0_H
#define C8051_SPI0_H

// tx/rx function for operation as master
void SPI0_putGetChars(void* buffer, short length);
// tx function for operation as master, rx is discarded, values in buffer remain
void SPI0_putChars(void* buffer, short length);

#endif
