// code by ntu ael

#include <stdio.h>

#include "hash_blob.h"

#include "shared_general.h"
#include "shared_pwrs.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"

#include "state_util.h"

// Transfer [XBee & PSAT data] (1 frame) to OBDH using polling method
char State_ferryXBee(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  short length;
  char value;
  switch (*subtask) {
  case 0: // sending request to check if XBee & PSAT housekeeping data is available
    StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_XBEE_TO_OBDH_READY, 1);
    ++*subtask;
    break;
  case 1: // waiting for reply
    if (StateUtil_peek(&length, &release) == reply_success) {
      if (i2c_rxbuffer[MCCIF_DATA_OFFSET + 0]) { // contains g_uc_xbee_to_obdh_data_ready
        // optional: to add in length to obtain....
        StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_PWRS_XBE_OFFSET, XDATA_PWRS_XBE_LENGTH);
        ++*subtask;
      } else
        release = 1; // else release state
    }
    break;
  case 2: // waiting for reply
    if (StateUtil_peek(&length, &release) == reply_success) {
      Hash_putBlob(i2c_rxbuffer, length);
      Recon_debugPrint(i2c_rxbuffer, length);
      printf("xbee frame received!\r\n");
      value = 0;
      StateUtil_pokeSetXdata(PWRS_ADDRESS, XDATA_XBEE_TO_OBDH_READY, 1, &value); // tear down flag
      ++*subtask; // go to next state to check if PSAT data is available
    }
    break;
  case 3: // wait for set reply
    if (StateUtil_peek(&length, &release) == reply_success)
      release = 1; // else release state
    break;
  }
  return release;
}
