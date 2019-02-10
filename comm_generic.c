// code by ntu ael

#include <string.h>

#include "C8051_SMBus.h"

#include "Velox_delay.h"

#include "comm_generic.h"

char* comm_txbuffer; // sending requests for hk data
char* comm_rxbuffer; // receives housekeeping data
short comm_failCount = 0;

// short instructions are typically sent within 1 ms.
// Delay_blockMs(COMM_SHORT_DELAY_MS); waits between 2-3 ms.
#define COMM_SHORT_DELAY_MS    2     // used to block briefly for convenience: so that short instructions to the comm hardware do not require an external wait mechanism
// ---

void Comm_sendChars(char address, unsigned char length) {
  SMBus_setTxBuffer(comm_txbuffer, length);
  SMBus_start(SMB_writeTo, address);
  Delay_blockMs(COMM_SHORT_DELAY_MS); // don't remove!
}

void Comm_getChars(char address, unsigned char length) {
  SMBus_setRxBuffer(comm_rxbuffer, length);
  SMBus_start(SMB_readFrom, address);
  Delay_blockMs(COMM_SHORT_DELAY_MS); // don't remove!
}

// initiate comm_txbuffer and comm_rxbuffer pointers
void Comm_init(char* i2c_txbuffer, char* i2c_rxbuffer) {
  comm_txbuffer = i2c_txbuffer;
  comm_rxbuffer = i2c_rxbuffer;
}
