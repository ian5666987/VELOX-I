// code by ntu ael

#include <string.h>

#include "shared_general.h"
#include "shared_adcs.h"

#include "hash_blob.h"

#include "obdh_globalvariables.h"
#include "obdh_constant.h"
#include "obdh_recon.h"

#include "state_util.h"

char State_ferryGps(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  short length;
  char value;
  switch (*subtask) {
  case 0: // sending request
    if (SDCard_isAvailable() && Subsystem_isAvailable(ADCS_ADDRESS)) { // only if the subsystem is ON we will get the data segment from them
      StateUtil_pokeRequestXdata(ADCS_ADDRESS, XDATA_ADCS_GPS_ISAVAILABLE, 1); // pushRequest
      ++*subtask;
    } else
      release = 1; // sd_card or ADCS is off -> release state
    break;
  case 1: // waiting for reply
    if (StateUtil_peek(&length, &release) == reply_success) {
      if (i2c_rxbuffer[MCCIF_DATA_OFFSET + 0]) { // gps frame is available, can continue to talk to adcs
        StateUtil_pokeRequestXdata(ADCS_ADDRESS, XDATA_ADCS_GPSBIN_OFFSET, XDATA_ADCS_GPSBIN_LENGTH);
        ++*subtask;
      } else
        release = 1; // no GPS frame is available -> release state
    }
    break;
  case 2: // waiting for reply
    if (StateUtil_peek(&length, &release) == reply_success) {
      Hash_putBlob(i2c_rxbuffer, length);
      Recon_debugPrint(i2c_rxbuffer, length);
      value = 0;
      StateUtil_pokeSetXdata(ADCS_ADDRESS, XDATA_ADCS_GPS_ISAVAILABLE, 1, &value); // tear down flag
      ++*subtask;
    }
    break;
  case 3: // wait for set reply
    if (StateUtil_peek(&length, &release) == reply_success)
      release = 1;
    break;
  }
  return release;
}
