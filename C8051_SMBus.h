// code by ntu ael

#ifndef C8051_SMBUS_H
#define C8051_SMBUS_H

#define SMB_writeTo    0
#define SMB_readFrom   1

// This section to set for proper I2C communication
// Device addresses (7 bits, lsb is a don't care)
// target
// Constant Declarations
#define  SMB_FREQUENCY        400000L        // Target SCL clock rate
#define  SMB_FREQUENCY_SLOW   200000L        // Target SCL clock rate
// This example supports between 10kHz and 100kHz

void SMBus_setRxBuffer(void* buffer, short length);
char SMBus_isRxComplete(short* length);
void SMBus_setTxBuffer(void* buffer, short length);
void SMBus_start(char mode, char target);
void SMBus_getStatusCount(void* buffer);
void SMBus_init(char address, long sys_clk);

#endif
