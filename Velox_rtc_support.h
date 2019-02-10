// code by ntu ael

#ifndef VELOX_RTC_SUPPORT_H
#define VELOX_RTC_SUPPORT_H

#include "rtc_generic.h"

unsigned long Rtc_toTai(RtcStamp* rtcStamp);
void Rtc_fromTai(RtcStamp* rtcStamp, unsigned long tai_s);

#endif
