// code by ntu ael
// adapted from CodeVision AVR by HP

#define  interrupt(x)     interrupt x

#include <c8051F120.h>

#include "C8051_UART0.h"

#define UART0_TX_TIMEOUT 1500 // expected minimum of 9600 baud rate. Timeout is twice of maximum time to transfer one byte in the specified baudrate
// the buffer and buffer size are set in the open function and shall not
// be changed until closing the uart
char* UART0_rx;
short UART0_rx_size;
char* UART0_tx;
short UART0_tx_size;

// these are private variables managed by the uart
short UART0_rx_uart;
short UART0_rx_main;
volatile short UART0_rx_available;

short UART0_tx_uart = 0;
short UART0_tx_main = 0;
volatile short UART0_tx_available = 0;

char UART0_rx_errorCount = 0;
char UART0_tx_errorCount = 0;

// private function
void UART0_resetRx() {
  bit ES0_SAVE = ES0;
  ES0 = 0;
  UART0_rx_uart = 0;
  UART0_rx_main = 0;
  UART0_rx_available = 0;
  ES0 = ES0_SAVE;
}

void UART0_open(char* rx_buffer, short rx_size, char* tx_buffer, short tx_size, char p_uc_SSTA0) {
  char SFRPAGE_SAVE = SFRPAGE;
  UART0_rx = rx_buffer;
  UART0_rx_size = rx_size;
  UART0_tx = tx_buffer;
  UART0_tx_size = tx_size;
  UART0_close();
  UART0_resetRx();

  SFRPAGE = UART0_PAGE;
  //SCON0 Register (UART0 Control)
  //Bits7–6:   SM00–SM10: Serial Port Operation Mode:
  //Bit5:   SM20: Multiprocessor Communication Enable.
  //Bit4:   REN0: Receive Enable.
  //Bit1:   TI0: Transmit Interrupt Flag.
  //Bit0:   RI0: Receive Interrupt Flag.
  SCON0 = 0x50; //  0101 0000
  //SSTA0 Register (UART0 Status and Clock Selection)
  //Bit7:   FE0: Frame Error Flag.*
  //Bit6:   RXOV0: Receive Overrun Flag.*
  //Bit5:   TXCOL0: Transmit Collision Flag.*
  //Bit4:   SMOD0: UART0 Baud Rate Doubler Enable
  //Bits3–2:   UART0 Transmit Baud Rate Clock Selection Bits
  //Bits1–0:   UART0 Receive Baud Rate Clock Selection Bits

  SSTA0 = p_uc_SSTA0; // typically 0000 1111 :: Clear all error flags, select Timer 4 overflow as TX and RX baud rate sources
  //Indicate TX0 ready
  TI0 = 0;
  RI0 = 0;
  ES0 = 1; // enable UART0 interrupt
  SFRPAGE = SFRPAGE_SAVE;
}

void UART0_close() {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = UART0_PAGE;
  ES0 = 0; //
  TI0 = 0;
  RI0 = 0;
  UART0_rx_available = 0;
  UART0_tx_uart = 0;
  SFRPAGE = SFRPAGE_SAVE;
}

short UART0_getRxAvailable() {
  short value;
  bit ES0_SAVE = ES0;
  ES0 = 0;
  value = UART0_rx_available;
  ES0 = ES0_SAVE;
  return value;
}

// returns false if not sufficient data in buffer
char UART0_pollChars(void* ptr, short length) {
  char* message = ptr;
  short count;
  if (length <= UART0_getRxAvailable()) {
    for (count = 0; count < length; ++count)
      message[count] = UART0_rx[(UART0_rx_main + count) % UART0_rx_size];
    return 1;
  }
  return 0;
}

void UART0_advance(short length) {
  bit ES0_SAVE = ES0;
  ES0 = 0;
  UART0_rx_available -= length;
  UART0_rx_main += length;
  UART0_rx_main %= UART0_rx_size;
  ES0 = ES0_SAVE;
}

void UART0_putChar(char value) {
  char SAVE_SFRPAGE;
  short delay = 0;
  bit ES0_SAVE;
  while (UART0_tx_size <= UART0_tx_available && delay < UART0_TX_TIMEOUT)
    ++delay;
  if (UART0_tx_size <= UART0_tx_available) {
    ++UART0_tx_errorCount;
    return;
  }
  // only comes here if UART0_tx_available < UART0_tx_size
  // ---
  ES0_SAVE = ES0;
  ES0 = 0;
  if (UART0_tx_available) {
    UART0_tx[UART0_tx_main] = value;
    ++UART0_tx_main;
    if (UART0_tx_main == UART0_tx_size)
      UART0_tx_main = 0;
    ++UART0_tx_available; //increase this before any transmission
  } else {
    SAVE_SFRPAGE = SFRPAGE;
    SFRPAGE = UART0_PAGE;
    ++UART0_tx_available; //increase this before any transmission
    SBUF0 = value;
    SFRPAGE = SAVE_SFRPAGE;
  }
  ES0 = ES0_SAVE;
}

void UART0_putChars(void* ptr, short length) {
  short count;
  char* message = (char*) ptr;
  for (count = 0; count < length; ++count)
    UART0_putChar(message[count]);
}

UART0_ISR() interrupt (4) {
  if (RI0 == 1) {
    RI0 = 0;
    // data will be loaded in SBUF0 if
    // RI0 == 0 && (SM20 == 1 => StopBit == 1)
    if (SSTA0 & 0xC0) { // 0xC0 = 1100 0000 frame error and receiver overflow
      ++UART0_rx_errorCount;
      SSTA0 &= 0x3F; // Clear FE0 & RXOV0 flags
    } else {
      UART0_rx[UART0_rx_uart] = SBUF0; // Read a character from UART0 Data Buffer
      ++UART0_rx_uart;
      if (UART0_rx_uart == UART0_rx_size)
        UART0_rx_uart = 0;
      ++UART0_rx_available;
    }
  }
  if (TI0 == 1) { // cause of interrupt: previous tx is finished
    TI0 = 0;
    if (SSTA0 & 0x20) { // tx collision error
      ++UART0_tx_errorCount;
      SSTA0 &= 0xDF;
    }
    --UART0_tx_available; // Decrease array size
    if (UART0_tx_available) { // If buffer not empty
      SBUF0 = UART0_tx[UART0_tx_uart]; // Transmit
      ++UART0_tx_uart; // Update counter
      if (UART0_tx_uart == UART0_tx_size)
        UART0_tx_uart = 0;
    }
  }
}

char UART0_getRxErrorCount() {
  return UART0_rx_errorCount;
}

char UART0_getTxErrorCount() {
  return UART0_tx_errorCount;
}

