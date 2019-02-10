// code by ntu ael

#define  interrupt(x)     interrupt x

#include <c8051F120.h>
#include <string.h>

#include "C8051_SMBus.h"

sbit SDA = P0 ^ 6; // SMBus on P0.6
sbit SCL = P0 ^ 7; // and P0.7

char* SMBus_rxBuffer = 0;
volatile short SMBus_rxCount; // counts the number of bytes received
volatile short SMBus_rxLengthMax = 0;
volatile char SMBus_rxIsDone = 0; // private (!) flag

char* SMBus_txBuffer = 0;
short SMBus_txCount;
volatile short SMBus_txLength = 0;
volatile char SMBus_target = 0;

volatile char SMBus_MT_retryCount = 0;
#define SMBUS_MT_RETRYLIMIT 20

char xdata SMBus_StatusCount[27];

#define SBMUS_INTERRUPT_DISABLE(EIE1_SAVE)    EIE1_SAVE = EIE1 & 0x02; EIE1 &= 0xFD;
#define SMBUS_INTTERUPT_ENABLE(EIE1_SAVE)     EIE1 |= EIE1_SAVE;

// function needs be called before SMBus reception is enabled
void SMBus_setRxBuffer(void* buffer, short length) {
  char EIE1_SAVE = EIE1 & 0x02;
  EIE1 &= 0xFD; // 1111 1101
  SMBus_rxBuffer = buffer;
  SMBus_rxLengthMax = length;
  EIE1 |= EIE1_SAVE;
}

char SMBus_isRxComplete(short* length) {
  char status;
  status = SMBus_rxIsDone;
  if (status) {
    SMBus_rxIsDone = 0;
    *length = SMBus_rxCount;
  }
  return status;
}

void SMBus_setTxBuffer(void* buffer, short length) {
  char EIE1_SAVE = EIE1 & 0x02;
  EIE1 &= 0xFD; // 1111 1101
  SMBus_txBuffer = buffer;
  SMBus_txLength = length;
  EIE1 |= EIE1_SAVE;
}

// mode==0 is write, mode==1 is read
// if any communication is presently ongoing, this function will stop the previous communication
void SMBus_start(char mode, char target) {
  char SFRPAGE_SAVE;
  char EIE1_SAVE; // 0000 0010
  switch (mode) {
  case SMB_writeTo:
    SMBus_MT_retryCount = 0; // reset write count
    break;
  case SMB_readFrom:
    SMBus_rxIsDone = 0; // this was added by jan when uplink of comm module became relevant
    break;
  default:
    return;
  }
  // ---
  EIE1_SAVE = EIE1 & 0x02; // 0000 0010
  EIE1 &= 0xFD; // 1111 1101
  SMBus_target = target | mode;
  EIE1 |= EIE1_SAVE;
  // ---
  SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = SMB0_PAGE;
  STA = 1;
  SFRPAGE = SFRPAGE_SAVE;
}

// buffer must hold at least 27 bytes
void SMBus_getStatusCount(void* buffer) {
  char EIE1_SAVE = EIE1 & 0x02;
  EIE1 &= 0xFD; // 1111 1101
  memcpy(buffer, SMBus_StatusCount, sizeof SMBus_StatusCount);
  EIE1 |= EIE1_SAVE;
}

void SMBus_init(char address, long sys_clk) {
  char SFRPAGE_SAVE = SFRPAGE;
  long smb_freq = sys_clk > 6000000L ? SMB_FREQUENCY : SMB_FREQUENCY_SLOW;
  SFRPAGE = SMB0_PAGE;
  SMB0CN = 0x07;
  SMB0CR = 257 - (float) (sys_clk / (8 * smb_freq)) - 0.18; // do not remove the 0.18 float TODO the float case is in the wrong place FIXME rounding is different from rounding in EXCEL!!!
  SMB0ADR = address;
  SMB0CN |= 0x40;
  EIE1 |= 0x02; // enable smbus interrupt, page 155
  SFRPAGE = SFRPAGE_SAVE;
  memset(SMBus_StatusCount, 0, sizeof SMBus_StatusCount);
}

SMBus_ISR() interrupt (7) {
  char value;
  // ---
  value = (SMB0STA >> 3) & 0x1F;
  if (value < sizeof SMBus_StatusCount)
    ++SMBus_StatusCount[value];
  // ---
  switch (SMB0STA) { // Status code for the SMBus

// MASTER RX --------------------------------------------------------------------------------
#define SMB_START 0x08 // START condition transmitted
#define SMB_RP_START 0x10           // (MT & MR) repeated START
  case SMB_START:
  case SMB_RP_START:
    SMB0DAT = SMBus_target; // load SMB0DAT with slave address + R/W.
    SMBus_txCount = 0;
    SMBus_rxCount = 0;
    STA = 0; // clear STA
    break;
    // ---
#define SMB_MTADDACK 0x18 // Slave address + W transmitted. ack received
  case SMB_MTADDACK:
    SMB0DAT = SMBus_txBuffer[SMBus_txCount]; // load SMB0DAT with data to be transmitted
    ++SMBus_txCount;
    break;
    // ---
#define SMB_MTADDNACK 0x20 // Slave address + W transmitted. NACK received
  case SMB_MTADDNACK:
    // TODO acknowledge poll to retry
    if (SMBus_MT_retryCount < SMBUS_MT_RETRYLIMIT) {
      STA = 1; // this means retry...
      ++SMBus_MT_retryCount;
    } else {
      STO = 1;
      SMBus_MT_retryCount = 0;
    }
    break;
// ---
#define SMB_MTDBACK 0x28 // data byte transmitted; ACK received
  case SMB_MTDBACK:
    // silabs code even has a STA = 1 here, but our version works perfectly fine
    if (SMBus_txCount < SMBus_txLength) {
      SMB0DAT = SMBus_txBuffer[SMBus_txCount]; // load SMB0DAT with data to be transmitted
      ++SMBus_txCount;
    } else
      STO = 1; // set STO
    break;
    // ---
#define SMB_MTDBNACK 0x30 // data byte transmitted; nack received
  case SMB_MTDBNACK:
    if (SMBus_MT_retryCount < SMBUS_MT_RETRYLIMIT) {
      STA = 1;
      ++SMBus_MT_retryCount;
    } else {
      STO = 1;
      SMBus_MT_retryCount = 0;
    }
    break;
    // ---
#define SMB_MTARBLOST 0x38 // arbitration lost
  case SMB_MTARBLOST:
    // that means that some other smbus client is allowed to send a message and we have to wait until that sending is complete
    break;
    // ---

// MASTER RX --------------------------------------------------------------------------------
#define SMB_MRADDACK 0x40 // Slave address + R transmitted; ACK received
  case SMB_MRADDACK:
    AA = SMBus_rxCount < (SMBus_rxLengthMax - 1); //changed after experiment with ISIS
    break;
    // ---
#define SMB_MRADDNACK 0x48 // Slave address + R transmitted; NACK received
  case SMB_MRADDNACK:
    STA = 1; // silabs STA = 1 only
    // TODO could use a threshold else STO=1
    break;
    // ---
#define SMB_MRDBACK 0x50 // data byte rec'vd; ACK transmitted
  case SMB_MRDBACK:
    SMBus_rxBuffer[SMBus_rxCount] = SMB0DAT; // protect from overflow not necessary since master controls end of transfer via AA
    ++SMBus_rxCount;
    AA = SMBus_rxCount < SMBus_rxLengthMax - 1; // so that for the last byte the case SMB_MRDBNACK is invoked
    break;
    // ---
#define SMB_MRDBNACK 0x58 // data byte rec'vd; NACK transmitted
  case SMB_MRDBNACK:
    SMBus_rxBuffer[SMBus_rxCount] = SMB0DAT; // protect from overflow not necessary since SMB_MRDBNACK entered only once for final byte of transfer
    ++SMBus_rxCount; // after this instruction we should have SMBus_rxCount == SMBus_rxLengthMax
    SMBus_rxIsDone = 1; // added (surprisingly late) in development because SMB_MRDBNACK indicates final char and marks successful reception
    STO = 1;
    AA = 1; // obdh and silabs set AA = 1
    break;
    // ---

// SLAVE RX --------------------------------------------------------------------------------
#define SMB_SROADACK 0x60 // Own slave address and Write received. ACK transmitted.
#define SMB_SRGADACK 0x70 // General call address received. ACK transmitted
  case SMB_SROADACK: // wait for data // fall through
  case SMB_SRGADACK: // wait for data
    SMBus_rxIsDone = 0;
    SMBus_rxCount = 0; // Reinitialize the data counters
    break;
    // ---
#define SMB_SROARBLOST 0x68 // Arbitration lost, own slave address + WRITE received. ACK transmitted.
#define SMB_SRGARBLOST 0x78 // Arbitration lost, general call address received. ACK transmitted.
  case SMB_SROARBLOST: // save current data for retry when bus is free // fall through
  case SMB_SRGARBLOST: // save current data for retry when bus is free
    // TODO there used to be a STA = 1; here
    STA = 1; // what is going on here !?
    break;
    // ---
#define SMB_SRODBACK 0x80 // Data byte received after addressed by own slave address + WRITE. ACK transmitted.
#define SMB_SRGDBACK 0x90 // Data byte received after addressed by general. ACK transmitted
  case SMB_SRODBACK: // read SMB0DAT. wait for next byte or stop // fall through
  case SMB_SRGDBACK: // read SMB0DAT. wait for next byte or stop
    value = SMB0DAT;
    if (SMBus_rxCount < SMBus_rxLengthMax)
      SMBus_rxBuffer[SMBus_rxCount] = value; // store data
    ++SMBus_rxCount;
    AA = 1;
    break;
    // ---
#define SMB_SRODBNACK 0x88 // Data byte received. NACK transmitted
#define SMB_SRGDBNACK 0x98 // Data byte received after general call address. NACK transmitted.
  case SMB_SRODBNACK: // Set STO to reset SMBus. // fall through
  case SMB_SRGDBNACK: // Set STO to reset SMBus.
    // TODO need to investigate this case!
    // STO = 1;
    AA = 1;
    break;
    // ---
#define SMB_SRSTOP 0xa0 // STOP or repeated START received
  case SMB_SRSTOP: // No action necessary.
    SMBus_rxIsDone = 1;
    break;

// SLAVE TX --------------------------------------------------------------------------------

#define SMB_STOADACK 0xa8 // Own slave address + READ received. ACK transmitted.
#define SMB_STDBACK 0xb8 // Data byte transmitted. ACK received.
  case SMB_STOADACK: // Load SMB0DAT with data to transmit. // fall through
  case SMB_STDBACK: // Load SMB0DAT with data to transmit.
    // case untested since the micro-controllers on Velox are never in SlaveTransmitter mode
    if (SMBus_txCount < SMBus_txLength) { // read from the TX buffer
      SMB0DAT = SMBus_txBuffer[SMBus_txCount];
      ++SMBus_txCount;
    } else {
      // error case: other party requests more bytes than expected
      SMB0DAT = 0;
    }
    break;
    // ---
#define SMB_STOARBLOST 0xb0 // Arbitration lost as master. Own address + READ received. ACK transmitted.
  case SMB_STOARBLOST:
    // case untested since the micro-controllers on Velox are never in SlaveTransmitter mode
    STA = 1; // Set STA bit to retry the transfer when the bus is free.
    break;
    // ---
#define SMB_STDBNACK 0xc0 // Data byte transmitted. NACK received.
  case SMB_STDBNACK: // wait for STOP
    // case untested since the micro-controllers on Velox are never in SlaveTransmitter mode
    SMB0DAT = 0 < SMBus_txCount ? SMBus_txBuffer[SMBus_txCount - 1] : 0;
    break;
    // ---
#define SMB_STDBLAST 0xc8 // Last data byte transmitted (AA=0). ACK received.
  case SMB_STDBLAST: // set STO to reset SMBus. // fall through
    // case untested since the micro-controllers on Velox are never in SlaveTransmitter mode
    break;
    // ---
#define SMB_SCLHIGHTO 0xd0 // SCL clock high timer per SMB0CR timed out.
#define SMB_BUS_ERROR 0x00 // Bus Error (illegal Start or Stop)
  case SMB_SCLHIGHTO: // set STO to reset SMBus. // fall through
  case SMB_BUS_ERROR: // set STO to reset SMBus.
    STO = 1; // not sure what the options are here, state typically does not occur
    break;
    // ---
#define SMB_IDLE 0xf8 // Idle
  case SMB_IDLE: // state does not set SI
    break;
    // All other status codes invalid.  Reset communication.
  default:
    // failure state: Reset communication
    SMB0CN &= ~0x40;
    SMB0CN |= 0x40;
    STA = 0;
    STO = 0;
    AA = 0;
    break;
  }
  SI = 0; // Clear interrupt flag
}
