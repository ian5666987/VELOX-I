// code by ntu ael

#include <stdio.h>
#include <string.h>

#include "c8051_SMBus.h"
#include "C8051_SMBusSpy.h"

#include "Velox_delay.h"

#include "mccif_spy.h"
#include "mccif_xdata.h"

#include "obdh_globalvariables.h"
#include "obdh_schedule.h"

#include "hash_manifest.h"

void Mccif_setCompleted_Override(short offset) {
  offset += 0;
  Schedule_handleAdd();
}

void State_timeout_Override(State* state) {
  State* dummy = state; // to suppress warnings
  // can take action here if desired
  printf("state timeout ");
}

// called every 2-4 minutes
void Uptime_storeVitals_Override() {
  Hash_writeManifest();
  printf("store vitals ");
}
