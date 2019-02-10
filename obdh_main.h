// code by ntu ael

#ifndef OBDH_MAIN_H
#define OBDH_MAIN_H

#define interrupt(x)     interrupt x

#include <c8051f120.h>
#include <stdio.h>
#include <string.h>
#include <intrins.h>

#include "C8051_adc.h"
#include "C8051_delay.h"
#include "C8051_extern_int.h"
#include "C8051_macros.h"
#include "C8051_mcutemp.h"
#include "C8051_oscillator.h"
#include "C8051_pca0.h"
#include "C8051_SMBus.h"
#include "C8051_SMBusSpy.h"
#include "C8051_SMBusTimer3.h"
#include "C8051_SPI0.h"
#include "C8051_SPI0_shared.h"
#include "C8051_UART1Timer1.h"
#include "C8051_UART0Timer4.h"
#include "C8051_watchdog.h"

#include "Velox_sfrstack.h"
#include "Velox_timertask.h"
#include "Velox_checksum.h"
#include "Velox_clock.h"
#include "Velox_uart.h"
#include "Velox_delay.h"
#include "Velox_watchdog.h"
#include "Velox_macros.h"
#include "Velox_meta.h"
#include "Velox_seconds.h"
#include "Velox_seconds.h"
#include "Velox_state.h"
#include "Velox_uptime.h"
#include "Velox_rtc_support.h"
#include "Velox_calendar.h"

#include "sdcard_redundant.h"
#include "comm_generic.h"
#include "rtc_generic.h"

#include "shared_general.h"
#include "shared_obdh.h"
#include "shared_pwrs.h"
#include "shared_adcs.h"
#include "shared_psat.h"

#include "mccif_active.h"
#include "mccif_buffer.h"
#include "mccif_local.h"
#include "mccif_spy.h"
#include "mccif_xdata.h"

#include "obdh_auth.h"
#include "obdh_cqt.h"
#include "obdh_debug.h"
#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_grace.h"
#include "obdh_port.h"
#include "obdh_schedule.h"
#include "obdh_systemconfig.h"
#include "obdh_timer.h"
#include "obdh_timekeeper.h"
#include "obdh_trxuv.h"

#include "state_downlink.h"
#include "state_cmdHandler.h"
#include "state_pwrsHk.h"
#include "state_adcsHk.h"
#include "state_ferryGps.h"
#include "state_scheduler.h"
#include "state_ferryCct.h"
#include "state_ferryXBee.h"
#include "state_radioUp.h"
#include "state_rtcSync.h"
#include "state_trxuvHk.h"
#include "state_powerCqt.h"
#include "state_wireUp.h"
#include "state_util.h"
#include "state_xdatSegGet.h"

// ---

extern Statetask g_statetask_pwrsHk;
extern Statetask g_statetask_adcsHk;
extern Statetask g_statetask_trxuvHk;
extern Statetask g_statetask_xdatSegGet;
extern Statetask g_statetask_wireUp;
extern Statetask g_statetask_radioUp;
extern Statetask g_statetask_cmdHandler;
extern Statetask g_statetask_scheduler;
extern Statetask g_statetask_downlink;
extern Statetask g_statetask_ferryGps;
extern Statetask g_statetask_ferryCam;
extern Statetask g_statetask_ferryXBee;
extern Statetask g_statetask_rtcSync;
extern Statetask g_statetask_powerCqt;

#endif
