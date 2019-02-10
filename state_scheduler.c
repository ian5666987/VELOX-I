// code by ntu ael

#include <string.h>
#include <stdio.h>

#include "mccif_active.h"
#include "mccif_xdata.h"

#include "shared_general.h"

#include "state_util.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_recon.h"

char State_scheduler(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  short length;
  char buffer[MCCIF_DATA_OFFSET + MCCIF_COMMAND_MAXLEN + 2];
  switch (*subtask) {
  case 0: // sending request to check if XBee & PSAT housekeeping data is available
    // Check if there are overdue schedules. The time will also be updated here!! Ideally, this is to be called every second
    if (Schedule_hasNext()) { // return 0 or 1 depending if message was prepared
      if (schedule[0].xdatSeg.system == mccif_localSystem) {
        Mccif_formatSet( //
            schedule[0].xdatSeg.system, //
            schedule[0].xdatSeg.offset, //
            schedule[0].xdatSeg.length, //
            schedule[0].arg, //
            buffer); // this could easily be done outside
        Mccif_localSet(buffer);
        Schedule_remove(0, 1);
        release = 1;
      } else {
        if (Subsystem_isAvailable(schedule[0].xdatSeg.system)) {
          StateUtil_pokeSetXdata(schedule[0].xdatSeg.system, //
              schedule[0].xdatSeg.offset, //
              schedule[0].xdatSeg.length, //
              schedule[0].arg);
          printf("scheduler waits... ");
          ++*subtask;
        } else
          release = 1; // subsystem not available, cannot proceed
      }
    } else
      release = 1;
    break;
  case 1:
    if (StateUtil_peek(&length, &release) == reply_success) {
      Schedule_remove(0, 1);
      release = 1;
    }
    break;
  }
  return release;
}
