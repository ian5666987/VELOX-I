// code by ntu ael
// adapted from CodeVision AVR by HP

#define interrupt(x) interrupt x

#include <c8051f120.h>
#include "C8051_UART1.h"

 // ?? not sure, no numbers given: expected minimum of 9600 baud rate. Timeout is twice of maximum time to transfer one byte in the specified baudrate
#define UART1_TX_TIMEOUT 2000 // was 1500 before

// the buffer and buffer size are set in the open function and shall not
// be changed until closing the uart
char* UART1_rx;
short UART1_rx_size;
char* UART1_tx;
short UART1_tx_size;

// these are private variables managed by the uart
short UART1_rx_uart = 0;
short UART1_rx_main = 0;
volatile short UART1_rx_available = 0;

// uart1 has no specific error flag
short UART1_tx_uart = 0;
short UART1_tx_main = 0;
volatile short UART1_tx_available = 0;

//char UART1_rx_errorCount = 0; // n.a.
char UART1_tx_errorCount = 0;

void UART1_open(char* rx_buffer, short rx_size, char* tx_buffer, short tx_size) {
  char SFRPAGE_SAVE = SFRPAGE;
  UART1_rx = rx_buffer;
  UART1_rx_size = rx_size;
  UART1_tx = tx_buffer;
  UART1_tx_size = tx_size;
  UART1_close();

  SFRPAGE = UART1_PAGE;

  //SCON1 Register (UART1 Control)
  //Bit7:   S1MODE: Serial Port 1 Operation Mode:
  //Bit6:   Unused.
  //Bit5:   MCE1: Multiprocessor Communication Enable.
  //Bit4:   REN0: Receive Enable.
  //Bit1:   TI1: Transmit Interrupt Flag.
  //Bit0:   RI1: Receive Interrupt Flag.
  SCON1 = 0x10; //  0001 0000

  TI1 = 0; // TX ready
  RI1 = 0; // RX ready
  EIE2 |= 0x40; // Set ES1 UART1 interrupt  1 - enabled, 0 - disabled

  SFRPAGE = SFRPAGE_SAVE;
}

void UART1_close() {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = UART1_PAGE;
  EIE2 &= 0xBF; // Set ES1 UART1 interrupt  1 - enabled, 0 - disabled
  TI1 = 0;
  RI1 = 0;
  UART1_rx_available = 0;
  UART1_tx_uart = 0;
  SFRPAGE = SFRPAGE_SAVE;
}

short UART1_getRxAvailable() {
  short value;
  char EIE2_SAVE = EIE2 & 0x40;
  EIE2 &= 0xBF; // Set ES1 = EIE2^6 UART1 interrupt  1 - enabled, 0 - disabled
  value = UART1_rx_available;
  EIE2 |= EIE2_SAVE;
  return value;
}

char UART1_pollChars(void* ptr, short length) {
  char* message = ptr;
  short count;
  if (length <= UART1_getRxAvailable()) {
    for (count = 0; count < length; ++count)
      message[count] = UART1_rx[(UART1_rx_main + count) % UART1_rx_size];
    return 1;
  }
  return 0;
}

void UART1_advance(short length) {
  char EIE2_SAVE = EIE2 & 0x40;
  EIE2 &= 0xBF; // Set ES1 = EIE2^6 UART1 interrupt  1 - enabled, 0 - disabled
  UART1_rx_available -= length;
  UART1_rx_main += length;
  UART1_rx_main %= UART1_rx_size;
  EIE2 |= EIE2_SAVE;
}

void UART1_putChar(unsigned char value) {
  char SFRPAGE_SAVE;
  char EIE2_SAVE;
  short delay = 0;
  // buffer is full, wait for the UART1 to transmit and free up buffer space (rather than discard)
  while (UART1_tx_size <= UART1_tx_available && delay < UART1_TX_TIMEOUT)
    ++delay;
  if (UART1_tx_size <= UART1_tx_available) {
    ++UART1_tx_errorCount;
    return;
  }
  // only comes here if UART1_tx_available < UART1_tx_size
  // ---
  EIE2_SAVE = EIE2 & 0x40;
  EIE2 &= 0xBF; // Set ES1 UART1 interrupt  1 - enabled, 0 - disabled
  if (UART1_tx_available) {
    UART1_tx[UART1_tx_main] = value; // Store in transmit array
    ++UART1_tx_main; // Update array's size
    // reach end of buffer! reset the pointer and the
    //  while loop at the front will ensure that the UART1_tx_main will not run over  g_us_tx1_ptr
    if (UART1_tx_main == UART1_tx_size)
      UART1_tx_main = 0;
    ++UART1_tx_available; // Update counter
  } else {
    SFRPAGE_SAVE = SFRPAGE;
    SFRPAGE = UART1_PAGE;
    ++UART1_tx_available; // Update counter
    SBUF1 = value; // Straight away transmit, dun even need to put in tx buffer
    SFRPAGE = SFRPAGE_SAVE; //Restore the SFRPAGE
  }
  EIE2 |= EIE2_SAVE;
}

void UART1_putChars(void* ptr, short length) {
  short count;
  char* message = (char*) ptr;
  for (count = 0; count < length; ++count)
    UART1_putChar(message[count]);
}

UART1_ISR() interrupt (20) {
  if (RI1 == 1) {
    RI1 = 0;
    // data will be loaded in SBUF0 if
    // RI1 == 0 && (SM20 == 1 => StopBit == 1)
    UART1_rx[UART1_rx_uart] = SBUF1; //Read a character from UART1 Data Buffer
    ++UART1_rx_uart;
    if (UART1_rx_uart == UART1_rx_size)
      UART1_rx_uart = 0;
    ++UART1_rx_available;
    // we do not check if buffer overflow
  }
  if (TI1 == 1) { // cause of interrupt: previous tx is finished
    TI1 = 0;
    --UART1_tx_available; //Decrease array size
    if (UART1_tx_available) { //If buffer not empty
      SBUF1 = UART1_tx[UART1_tx_uart]; //Transmit
      ++UART1_tx_uart; //Update counter
      if (UART1_tx_uart == UART1_tx_size)
        UART1_tx_uart = 0;
    }
  }
}

char UART1_getTxErrorCount() {
  return UART1_tx_errorCount;
}

