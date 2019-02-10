// code by ntu ael

#include <string.h>
//#include <stdio.h>

#include "shared_general.h"
#include "shared_pwrs.h"

#include "hash_blob.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"

#include "state_util.h"

char State_ferryCct(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  char value;
  short length;
  switch (*subtask) {
  case 0: // sending request
    if (SDCard_isAvailable()) {
      StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_PWRS_CAM_AVAILABLE, 1);
      ++*subtask;
    } else
      release = 1;
    break;
  case 1: // waiting for reply
    if (StateUtil_peek(&length, &release) == reply_success) {
      if (i2c_rxbuffer[MCCIF_DATA_OFFSET + 0]) { // contains cct_isAvailable
        StateUtil_pokeRequestXdata(PWRS_ADDRESS, XDATA_PWRS_BIN_OFFSET, XDATA_PWRS_BIN_LENGTH);
        ++*subtask;
      } else
        release = 1;
    }
    break;
  case 2: // waiting for reply
    if (StateUtil_peek(&length, &release) == reply_success) {
      Hash_putBlob(i2c_rxbuffer, length);
      Recon_debugPrint(i2c_rxbuffer, length);
      value = 0;
      StateUtil_pokeSetXdata(PWRS_ADDRESS, XDATA_PWRS_CAM_AVAILABLE, 1, &value);
      ++*subtask;
    }
    break;
  case 3:
    if (StateUtil_peek(&length, &release) == reply_success)
      release = 1;
    break;
  }
  return release;
}
