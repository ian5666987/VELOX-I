// code by ntu ael

#include "mccif_local.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"

#include "state_util.h"

extern Statetask g_statetask_downlink;

void StateUplink_releaseAck(); // defined in radioUp

char State_wireUp(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  char updated;
  short length;
  if (!g_c_sendAck_flag || g_statetask_downlink.task.mode == timertask_disabled) { // TODO not the final criteria
    switch (*subtask) {
    case 0: // sending request
      updated = Mccif_localPoll();
      if (updated) {
      	isRadioUp = 0; //ian 20140310: set this as zero to indicate wired uplink!
        if (g_sendAck.xdatSeg.system == mccif_localSystem)
          release = 1;
        else
          ++*subtask;
      } else
        release = 1;
      break;
    case 1:
      if (StateUtil_peek(&length, &release) == reply_success) {
        StateUplink_releaseAck(); // indicate to gnd station that uplink command has been received and reached target
        release = 1;
      }
      break;
    }
  } else
    release = 1;
  return release;
}

