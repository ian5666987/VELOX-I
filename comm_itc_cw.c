// code by ntu ael

#include <string.h>

#include "comm_generic.h"
#include "comm_internal.h"

#define ITC_ADDRESS 0xA2

// manual 5.2 page 39:

// DO NOT call this with buffer==i2c_txbuffer!
// use in combination with Comm_txAx25AndCwGet, to obtain if Comm_txCwPut was successful
#define ITC_TX_CW_MESSAGE    0x12  // ISIS ITC send CW message
void Comm_txCwPut(void* buffer, short length) {
  comm_txbuffer[0] = ITC_TX_CW_MESSAGE; // send CW frame and getting number of frames in rx buffer
  memcpy(&comm_txbuffer[1], buffer, length);
  Comm_sendChars(ITC_ADDRESS, 1 + length); //WARNING: if
}

