// code by ntu ael

#include <string.h>

#include "Velox_checksum.h"
#include "Velox_macros.h"
#include "Velox_uptime.h"

#include "mccif_xdata.h"

#include "shared_obdh.h"

#include "obdh_auth.h"
#include "obdh_globalvariables.h"

#define   AUTH_MASK      0xC1A551CAL
#define   AUTH_SEED_HI   0x1CED
#define   AUTH_SEED_LO   0xA1A5

void Auth_init() {
  memset(&g_auth, 0, sizeof g_auth);
}

char Auth_isPermitted_Override(void* buffer) {
  char* message = (char*) buffer;
  char success;
  short offset;
  if (g_slow_gndPass.mode == slowtask_executeOnce) // this means still active
    return 1;
  success = 1;
  success &= extractFromBuffer(unsigned char, message + MCCIF_SYSTEM_OFFSET) == mccif_localSystem;
  offset = extractFromBuffer(short, message + MCCIF_OFFSET_OFFSET);
  offset -= XDATA_OBDH_PAS_OFFSET;
  success &= 0 <= offset && offset < 2; // control, unlock
  success &= extractFromBuffer(unsigned char, message + MCCIF_LENGTH_OFFSET) <= 5 - offset;
  return success;
}

void Auth_localNotify_Override() {
  switch (g_auth.control) {
  case auth_normal:
    Slowtask_fromNow(&g_slow_gndPass);
    break;
  case auth_request:
    memcpy(&g_auth.keycode, &g_sendAck.uplinkId, sizeof g_auth.keycode);
    g_auth.keycode.value ^= AUTH_MASK;
    g_auth.keycode.array[0] = Checksum_publish(Checksum_update(&g_auth.keycode, sizeof g_auth.keycode, AUTH_SEED_HI));
    g_auth.keycode.array[1] = Checksum_publish(Checksum_update(&g_auth.keycode, sizeof g_auth.keycode, AUTH_SEED_LO));
    g_auth.unlock = g_auth.keycode.value ^ 1; // important: this instructions has to guarantee inequality of unlock and keycode
    g_auth.control = auth_inject; // advance as courtesy: after requesting gnd pass, gnd station only has to transmit single parameter "unlock"
    break;
  case auth_inject:
    if (g_auth.unlock == g_auth.keycode.value) {
      if (!g_slow_gndPass.mode) // gnd pass was disabled previously
        g_rtHk.mask_now = g_rtHk.template;
      Slowtask_setExecuteOnce(&g_slow_gndPass, g_slow_gndPass.delay);
      g_auth.control = auth_normal;
    }
    break;
  case auth_expire:
    Slowtask_setDisabled(&g_slow_gndPass);
    g_rtHk.mask_now = 0; // stop any downlink of real-time housekeeping data
    break;
  }
}
