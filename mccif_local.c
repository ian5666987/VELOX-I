// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "Velox_checksum.h"
#include "Velox_uart.h"
#include "Velox_macros.h"

#include "mccif_xdata.h"
#include "mccif_local.h"

char mccif_port = 0;
char mccif_localSystem = 0; // localhost

/** set uart port and subsystem id */
void Mccif_localInit(char port, char system) {
  mccif_port = port;
  mccif_localSystem = system;
}

/** transmit memory image to mcc */
void Mccif_localSend(short offset, short length, void* buffer) {
  short total; // header + length + checksum
  total = Mccif_formatContent(offset, length, buffer);
  Uart_putChars(mccif_port, buffer, total);
}

/** checks uart port for commands and if matching passes them to Mccif_localHandle_Override */
char Mccif_localPoll() {
  char message[MCCIF_DATA_OFFSET + MCCIF_COMMAND_MAXLEN + 2]; // name required for macro
  short length;
  char updated = 0;
  char success; // name required for macro
  char xdata* ptr = 0;
  while (Uart_pollChars(mccif_port, message, MCCIF_DATA_OFFSET)) { // poll for header: [XDAT 1|2 system offset length]
    success = memcmp(message, mccif_xdatMarker, sizeof mccif_xdatMarker) == 0;
    success &= message[4] == MCCIF_ID_SET || message[4] == MCCIF_ID_REQUEST; // preliminary condition
    // offset is ignored
    length = extractFromBuffer(unsigned char, message + MCCIF_LENGTH_OFFSET);
    success &= 0 < length && length <= MCCIF_COMMAND_MAXLEN;
    if (success) {
      if (Uart_pollChars(mccif_port, message, MCCIF_DATA_OFFSET + length + 2)) {
        success &= Checksum_isValid(message, MCCIF_DATA_OFFSET + length + 2);
        Uart_advance(mccif_port, MCCIF_DATA_OFFSET + length + 2);
        if (success) {
          updated |= Mccif_localHandle_Override(message);
          break; // TODO jan added this break relatively late into development -> still needs testing on all subsystems
        }
      } else // insufficient characters in buffer
        break;
    }
    if (!success) // message corrupt
      Uart_advance(mccif_port, 1);
  }
  return updated;
}
