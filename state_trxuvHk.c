// code by ntu ael

#include "comm_imc.h"
#include "comm_itc.h"

#include "state_util.h"

// 2ms is proven to be working well so long as the ISIS_DELAY_MS in comm_generic.c is set to be 2ms as well
// triple the value just to be safe in case ISIS "clock stretching" goes wild...
#define TRXUVHK_PREPARE_REPLY_MS    6
#define TRXUVHK_NUMEL               4

volatile unsigned char trxuvHk_cycle = 0;

char Trxuv_isAvailable();

char State_trxuvHk(char* subtask) {
  char release = 0; // default return choice: do not release state, unless specific set (see release = 1; instruction below)
  if (Trxuv_isAvailable()) {
    switch (*subtask) {
    case 0: // contact trxuv
      switch (trxuvHk_cycle) {
      case 0:
        Comm_allTelemetryPut();
        break;
      case 1:
        Comm_rxUptimePut();
        break;
      case 2:
        Comm_txUptimePut();
        break;
      case TRXUVHK_NUMEL - 1: // 3
        Comm_txStatusPut();
        break;
      }
      Timertask_setExecuteOnce(&g_stateTask, TRXUVHK_PREPARE_REPLY_MS);
      ++*subtask;
      break;
    case 1: // demand reply from trxuv
      if (Timertask_isReady(&g_stateTask)) {
        switch (trxuvHk_cycle) {
        case 0:
          Comm_allTelemetryGet();
          break;
        case 1:
          Comm_rxUptimeGet();
          break;
        case 2:
          Comm_txUptimeGet();
          break;
        case TRXUVHK_NUMEL - 1: // 3
          Comm_txStatusGet();
          break;
        }
        ++trxuvHk_cycle;
        trxuvHk_cycle %= TRXUVHK_NUMEL;
        release = 1;
      }
      break;
    }
  } else { // if comm not available these are the suggested default values
    g_itc.tx_status = 0;
    g_itc.tx_available = 0;
    release = 1;
  }
  return release;
}
