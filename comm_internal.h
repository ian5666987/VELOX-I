// code by ntu ael

#ifndef COMM_INTERNAL_H
#define COMM_INTERNAL_H

void Comm_sendChars(char address, unsigned char length);

#define  Comm_sendChar(value, address) \
  comm_txbuffer[0] = value; \
  Comm_sendChars(address, 1)

void Comm_getChars(char address, unsigned char length);

extern char* comm_txbuffer; // sending requests for hk data
extern char* comm_rxbuffer; // receives housekeeping data

#endif
