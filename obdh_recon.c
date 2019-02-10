// code by ntu ael

#include "sdcard_redundant.h"

#include "Velox_timertask.h"
#include "Velox_uart.h"
#include "Velox_uptime.h"

#include "shared_general.h"
#include "shared_pwrs.h"

#include "comm_generic.h"

#include "obdh_globalvariables.h"

#define  PWRS_STARTUP_DELAY_SEC    2L  // require pwrs on for 2+1=3 sec until contact
#define  ADCS_STARTUP_DELAY_SEC    2  // require adcs on for 2+1=3 sec until contact
// ---

char Subsystem_isAvailable(char address) {
  switch (address) {
  case OBDH_ADDRESS:
    return 1;
  case PWRS_ADDRESS:
    return PWRS_STARTUP_DELAY_SEC < uptime.session;
  case ADCS_ADDRESS:
    return (g_us_obdh_channel_status & CHANNEL_MASK_ADS) && !g_c_adcsIsReadyIn; // brackets are required
  }
  return 0;
}

char SDCard_isAvailable() {
  return g_sdcard.isAvailable;
}

extern Statetask g_statetask_ferryCam;
extern Statetask g_statetask_ferryXBee;

void Recon_ChannelStatusUpdated_Event() {
  unsigned short mask;
  if ((g_us_obdh_channel_status & CHANNEL_MASK_ADS) == 0)
    g_c_adcsIsReadyIn = ADCS_STARTUP_DELAY_SEC;
  // ---
  mask = CHANNEL_MASK_SBC | CHANNEL_MASK_CSFB;
  if ((g_us_obdh_channel_status & mask) == mask)
    Timertask_setRepeated(&g_statetask_ferryCam.task, g_statetask_ferryCam.task.delay);
  else
    Timertask_setDisabled(&g_statetask_ferryCam.task);
  // ---
  mask = CHANNEL_MASK_XBEE;
  if ((g_us_obdh_channel_status & mask) == mask)
    Timertask_setRepeated(&g_statetask_ferryXBee.task, g_statetask_ferryXBee.task.delay);
  else
    Timertask_setDisabled(&g_statetask_ferryXBee.task);
  // ---
}

extern Timertask g_task_mccif;

char Recon_isDebugPrintEnabled() {
  return g_task_mccif.mode;
}

void Recon_debugPrint(void* buffer, short length) {
  if (g_task_mccif.mode)
    Uart_putChars(MCCIF_UART, buffer, length);
}
