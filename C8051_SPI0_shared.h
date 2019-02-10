// code by ntu ael

#ifndef C8051_SPI0_SHARED_H
#define C8051_SPI0_SHARED_H

#define  ESPI0_MASK   0x01

void SPI0_init(long sys_clk, long f_sck_max, unsigned char spi0cfg, short p_timeout);
short SPI0_getFailureCount();

#endif
