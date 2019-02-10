// code by ntu ael

#include <c8051F120.h>
#include <string.h>

#include "Velox_checksum.h"
#include "Velox_delay.h"
#include "Velox_state.h"
#include "Velox_uart.h"

#include "shared_general.h"
#include "shared_obdh.h"

#include "hash_blob.h"

#include "obdh_constant.h"
#include "obdh_cqt.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"

extern short g_s_BinaryId;
extern char g_c_BinaryType;
extern short g_us_BinaryFrame;
extern char g_c_BinaryContent[CQT_RAWDATA];

#define CQT_FRAME_LIMIT      1400
#define CQT_BYTES_LIMIT    248200L    // (1400*213+50000) Allow 50kB of "trash" bytes
// ---

// function called once before entering the main loop to initialize the most important variables
void Cqt_init() {
  memset(&g_cqtInfo, 0, sizeof g_cqtInfo);
  g_cqtInfo.runtime.delay = 1200; // 20 minutes
  g_cqtInfo.haltBelow = 50; // mA
  g_cqtInfo.txWait = 25; // ms to wait for entire frame to arrive
  g_cqtInfo.address = 0x10;
  g_cqtInfo.state = cqt_idle; // initialize CQT handler not to do anything unless specified otherwise
  g_c_BinaryType = blobtype_cqt;
}

// private function
void Cqt_clearRxBuffer() {
  unsigned char count;
  char value;
  for (count = 0; count < CQT_UART_RX_SIZE; ++count)
    if (Uart_pollChars(CQT_UART, &value, 1)) {
      Uart_advance(CQT_UART, 1);
      ++g_cqtInfo.byteCount;
    } else
      break;
}

void Cqt_startUp() {
  ++g_s_BinaryId; // ideally this is incremented by one for each time we switch on cqt and never be the same during all the mission. However, this can also be set by the gnd station
  g_us_BinaryFrame = 0; // incremented after a valid frame is stored
  // ---
  g_cqtInfo.byteCount = 0; // defined in the obdh_globalvariables.c, just to count the total number of bytes received from CQT. We are not interested to know the total number of bytes received.
  g_cqtInfo.framesRejected = 0; // this is to limit the no of stored frame for the CQT
  Runtime_fromNow(&g_cqtInfo.runtime);
}

// 0x10 0x00 GET_FRAME (for retry in case of some fault)
// 0x10 0x01 ACK_FRAME
void Cqt_sendCommand(char valid) {
  Uart_putChars(CQT_UART, &g_cqtInfo.address, 1); // the address is now a global variable, can be changed by CQT
  Uart_putChars(CQT_UART, &valid, 1); // if valid == 0 -> GET_FRAME, otherwise ACK_FRAME
}

// function advances uart buffer by CQT_RAWDATA_AND_CRC number of bytes
// then, performs crc check
// if crc check is positive, the frame is stored on the sd card
// the result of the crc check is returned
char Cqt_processFrame() {
  char valid;
  short length;
  Uart_advance(CQT_UART, CQT_RAWDATA_AND_CRC);
  g_cqtInfo.byteCount += CQT_RAWDATA_AND_CRC;
  Cqt_clearRxBuffer();
  // ---
  valid = Checksum_isValid(&g_c_BinaryContent, CQT_RAWDATA_AND_CRC); // check if the checksum is valid
  if (valid) {
    length = Mccif_formatContent(XDATA_OBDH_CQT_OFFSET, XDATA_OBDH_CQT_LENGTH, xdata_txbuffer);
    ES0 = 0; // disable uart0 to rule out cqt from messing with the spi process
    Hash_putBlob(xdata_txbuffer, length); // commit frame to Hash_putBlob, it will internally prevent other payload data from being deleted
    ES0 = 1; // re-enable uart0
    Recon_debugPrint(xdata_txbuffer, length);
    ++g_us_BinaryFrame;
  } else
    ++g_cqtInfo.framesRejected;
  return valid;
}

extern Statetask g_statetask_powerCqt;

// cqt sends content and checksum. the binary header 2+1+2 is defined by obdh.
void Cqt_handler() {
  switch (g_cqtInfo.state) {
  case cqt_idle:
    break;
  case cqt_requestPowerUp:
    g_statetask_powerCqt.task.mode = timertask_repeated;
    // pwrsHk state takes over cqt control, next state => cqt_powerUpGiven
    break;
  case cqt_powerUpGiven:
    g_statetask_powerCqt.task.mode = timertask_disabled;
    Timertask_setExecuteOnce(&g_cqtInfo.task, 8000); // check 5 sec from now, if channel status of cqt is still up
    g_cqtInfo.state = cqt_confirmStable;
    break;
  case cqt_confirmStable:
    if (Timertask_isReady(&g_cqtInfo.task)) // after 10 sec
      if (g_us_obdh_channel_status & CHANNEL_MASK_CQT) { // check if cqt channel is really on
        Cqt_startUp(); // proceed with cqt mission
        Cqt_clearRxBuffer();
        g_cqtInfo.state = cqt_listen;
      } else
        g_cqtInfo.state = cqt_requestPowerDown; // fault must have occurred, mission impossible
    break;
  case cqt_listen:
    if (Uart_pollChars(CQT_UART, &g_c_BinaryContent, 1)) // is there at least one byte received?
      if (Uart_pollChars(CQT_UART, &g_c_BinaryContent, CQT_RAWDATA_AND_CRC)) // is there are CQT_CONTENT bytes available
        Cqt_sendCommand(Cqt_processFrame());
      else {
        Timertask_setExecuteOnce(&g_cqtInfo.task, g_cqtInfo.txWait); // if there is not enough content, delay and later check again // 211 * 9 / 115200 => 16.48 ms
        g_cqtInfo.state = cqt_delayedRx;
      }
    if (g_cqtInfo.current < g_cqtInfo.haltBelow || // normal condition: cqt powers down internally, and obdh notices that via the current reading
        Runtime_hasExpired(&g_cqtInfo.runtime) || // fault condition: timeout e.g. after 20 min of operation
        CQT_FRAME_LIMIT <= g_us_BinaryFrame || // fault condition: total limit of frames reached
        CQT_BYTES_LIMIT <= g_cqtInfo.byteCount) // fault condition: total limit of bytes received
      g_cqtInfo.state = cqt_requestPowerDown;
    break;
  case cqt_delayedRx:
    if (Timertask_isReady(&g_cqtInfo.task)) { // if the timertask is ready, listen one more time
      if (Uart_pollChars(CQT_UART, &g_c_BinaryContent, CQT_RAWDATA_AND_CRC)) // is there are CQT_CONTENT bytes available
        Cqt_sendCommand(Cqt_processFrame());
      else { // insufficient bytes at this point
        Cqt_clearRxBuffer();
        ++g_cqtInfo.framesRejected;
        Cqt_sendCommand(0);
      }
      g_cqtInfo.state = cqt_listen;
    }
    break;
  case cqt_requestPowerDown:
    g_statetask_powerCqt.task.mode = timertask_repeated;
    // pwrsHk state takes over cqt control, next state => cqt_powerDownGiven
    break;
  case cqt_powerDownGiven:
    if (g_us_obdh_channel_status & CHANNEL_MASK_CQT) // channel is still high
      g_cqtInfo.state = cqt_requestPowerDown; // retry power off signal again
    else {
      g_statetask_powerCqt.task.mode = timertask_disabled; // power off was successful
      Cqt_clearRxBuffer();
      g_cqtInfo.state = cqt_idle;
    }
    break;
  default:
    g_cqtInfo.state = cqt_idle; //back to idle state whenever unknown state is detected
    break;
  }
}
