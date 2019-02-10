// code by ntu ael

#include <c8051F120.h>

#include "shared_general.h"
#include "shared_pwrs.h"

#include "obdh_cqt.h"

#include "state_util.h"

// the function Cqt_handler() activates and deactivates State_powerCqt
char State_powerCqt(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  short length;
  unsigned short channel;
  switch (*subtask) {
  case 0:
    if (g_cqtInfo.state == cqt_requestPowerUp) {
      channel = CHANNEL_MASK_CQT;
      StateUtil_pokeSetXdata(PWRS_ADDRESS, XDATA_PWRS_CH_EXTREQON_OFFSET, sizeof channel, &channel);
      ++*subtask;
    } else if (g_cqtInfo.state == cqt_requestPowerDown) {
      ES0 = 0;
      channel = CHANNEL_MASK_CQT;
      StateUtil_pokeSetXdata(PWRS_ADDRESS, XDATA_PWRS_CH_EXTREQOFF_OFFSET, sizeof channel, &channel);
      ++*subtask;
    } else
      release = 1;
    break;
  case 1:
    if (StateUtil_peek(&length, &release) == reply_success) {
      ES0 = g_cqtInfo.state == cqt_requestPowerUp; // enable / disable serial port // TODO still to compile/flash/test the line above
      ++g_cqtInfo.state; // nasty exploit: simply advance g_cqtInfo.state
      release = 1;
    }
    break;
  }
  return release;
}
