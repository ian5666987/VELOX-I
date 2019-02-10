// code by ntu ael

#ifndef C8051_MACROS_H
#define C8051_MACROS_H

// the #define's in this header file are intended for use in application layer
// the c8051 files should not depend on this header file

// all masks taken from datasheet page 155

#define   IE_TIMER0_MASK  0x02  // IE.1
//#define   IE_UART0_MASK   0x10  // IE.4
#define   IE_TIMER2_MASK  0x20  // IE.5

#define   EIE1_PCA_MASK   0x08  // EIE1.3

#endif
