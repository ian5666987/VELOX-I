// code by ntu ael

#include <stdio.h>
#include <string.h>

#include "C8051_SMBus.h"

#include "comm_generic.h"
#include "comm_internal.h"
#include "comm_imc.h"

#define IMC_ADDRESS 0xA0

// manual 5.2 page 18:

#define IMC_RESET     0xAA // ISIS IMC reset
void Comm_rxReset() {
  Comm_sendChar(IMC_RESET, IMC_ADDRESS);
}

// manual 5.2 page 19:

#define IMC_RX_FRAME_NUMEL    0x21   // ISIS IMC get no of Frame in receive buffer
void Comm_rxQueueSizePut() {
  // 6.1.2.2 get number of frames in rx buffer
  Comm_sendChar(IMC_RX_FRAME_NUMEL, IMC_ADDRESS);
}

char Comm_rxQueueSizeGet(char* numel) {
  short length;
#define IMC_RX_FRAMES_REPLY   1
  Comm_getChars(IMC_ADDRESS, IMC_RX_FRAMES_REPLY);
  if (SMBus_isRxComplete(&length))
    if (length == IMC_RX_FRAMES_REPLY) {
      *numel = comm_rxbuffer[0];
      return 1;
    }
  *numel = 0;
  ++comm_failCount;
  return 0;
}

// manual 5.2 page 20:

#define IMC_RX_FRAME_GET    0x22 // ISIS IMC get frame from receive buffer
void Comm_rxFramePut() {
  Comm_sendChar(IMC_RX_FRAME_GET, IMC_ADDRESS);
}

char Comm_rxFrameGet(void* buffer, char* numel) {
#define COMM_RX_FRAME_REPLY     17
#define COMM_RX_FRAME_CONTENT   16
#if COMM_RX_FRAME_CONTENT + 1 != COMM_RX_FRAME_REPLY
#error "out of sync: rx frame get"
#endif
  short length;
  Comm_getChars(IMC_ADDRESS, COMM_RX_FRAME_REPLY);
  if (SMBus_isRxComplete(&length))
    if (length == COMM_RX_FRAME_REPLY) {
      *numel = comm_rxbuffer[0];
      if (0 < *numel && *numel <= COMM_RX_FRAME_CONTENT) { // velox-i uplinks are typically 7-16 bytes long
        memcpy(buffer, comm_rxbuffer + 1, COMM_RX_FRAME_CONTENT); // pad with zeros
        return 1;
      }
    }
  ++comm_failCount;
  return 0;
}

// manual 5.2 page 21

#define IMC_RX_FRAME_DROP    0x24 // ISIS IMC remove frame from receive buffer
void Comm_rxFrameDrop() {
  Comm_sendChar(IMC_RX_FRAME_DROP, IMC_ADDRESS);
}

// manual 5.2 (see comm_deprec)
// page 22: rx doppler offset
// page 23: rx signal strength
// page 24: tx reflected power
// page 25: tx forward power
// page 26: tx current consumption
// page 27: rx current consumption
// page 28: power amplifier temperature
// page 29: power bus voltage

// manual 5.2 page 30, 31:

#define IMC_GET_ALL_TELEMETRY    0x1A   // ISIS IMC measure all data
void Comm_allTelemetryPut() {
  // 6.1.2.13 measure all telemetry channels
  Comm_sendChar(IMC_GET_ALL_TELEMETRY, IMC_ADDRESS);
}

char Comm_allTelemetryGet() {
  short length;
  char count;
  char temp;
  char* bytes;
#define COMM_ALL_TM_REPLY   16
  Comm_getChars(IMC_ADDRESS, COMM_ALL_TM_REPLY);
  if (SMBus_isRxComplete(&length))
    if (length == COMM_ALL_TM_REPLY) {
      bytes = (char*) &g_imc.doppler_offset;
      memcpy(bytes, comm_rxbuffer, COMM_ALL_TM_REPLY);
      for (count = 0; count < COMM_ALL_TM_REPLY; count += 2) {
        temp = bytes[count];
        bytes[count] = bytes[count + 1];
        bytes[count + 1] = temp;
      }
      return 1;
    }
  ++comm_failCount;
  return 0;
}

// manual 5.2 (see comm_deprec)
// page 32: power bus voltage
// page 32: rx current consumption
// page 32: tx current consumption

// manual 5.2 page 33:

#define IMC_UPTIME    0x40   // ISIS IMC report receiver uptime
void Comm_rxUptimePut() {
  // 6.1.2.15 get rx uptime
  Comm_sendChar(IMC_UPTIME, IMC_ADDRESS);
}

char Comm_rxUptimeGet() {
  short length;
#define COMM_RX_UPTIME_REPLY 3
  Comm_getChars(IMC_ADDRESS, COMM_RX_UPTIME_REPLY);
  if (SMBus_isRxComplete(&length))
    if (length == COMM_RX_UPTIME_REPLY) {
      memcpy(&g_imc.rx_uptime_minutes, comm_rxbuffer, COMM_RX_UPTIME_REPLY);
      return 1;
    }
  ++comm_failCount;
  return 0;
}
