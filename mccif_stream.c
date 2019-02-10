// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "Velox_checksum.h"
#include "Velox_uart.h"

#include "mccif_xdata.h"

void Mccif_downlinkStream(short index, short count, short length, void* msg) {
  char content[10];
  char* buffer = (char*) msg;
  unsigned short l_us_crc = 0;
  content[0] = 'S';
  content[1] = 'T';
  content[2] = 'R';
  content[3] = 'E';
  content[4] = index >> 8; // length hi
  content[5] = index; // length lo
  content[6] = count >> 8; // length hi
  content[7] = count; // length lo
  content[8] = length >> 8; // length hi
  content[9] = length; // length lo

  l_us_crc = Checksum_update(content, sizeof content, l_us_crc);
  Uart_putChars(mccif_port, content, sizeof content);

  l_us_crc = Checksum_update(buffer, length, l_us_crc);
  Uart_putChars(mccif_port, buffer, length);

  l_us_crc = Checksum_publish(l_us_crc);
  Uart_putChars(mccif_port, &l_us_crc, 2);
}
