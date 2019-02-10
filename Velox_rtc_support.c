// code by ntu ael

#include <string.h>

#include "Velox_calendar.h"

#include "rtc_generic.h"

// min 1957-12-31/23:59:25 = 0, max 2094-02-06/06:27:40 = -1

// private
void Rtc_toCalendar(RtcStamp* rtcStamp, Calendar* calendar) {
  calendar->year = 1900 + twoDec2oneHex(rtcStamp->yearMod100);
  if (memcmp(calendar, calendar_limit_lo, SIZEOF_CALENDAR) < 0) // below lower limit
    calendar->year += 100;
  calendar->month = twoDec2oneHex(rtcStamp->month);
  calendar->day = twoDec2oneHex(rtcStamp->day);
  calendar->hour = twoDec2oneHex(rtcStamp->hour);
  calendar->minute = twoDec2oneHex(rtcStamp->minute);
  calendar->second = twoDec2oneHex(rtcStamp->second);
}

// used when reading the time from RTC
unsigned long Rtc_toTai(RtcStamp* rtcStamp) {
  Calendar calendar;
  Rtc_toCalendar(rtcStamp, &calendar);
  return Calendar_toTai(&calendar);
}

// private
void Rtc_fromCalendar(RtcStamp* rtcStamp, Calendar* calendar) {
  unsigned char yearMod100;
  rtcStamp->centis = 0;
  rtcStamp->second = oneHex2twoDec(calendar->second);
  rtcStamp->minute = oneHex2twoDec(calendar->minute);
  rtcStamp->hour = oneHex2twoDec(calendar->hour);
  rtcStamp->weekday = 0x07; // weekday seems to be irrelevant for the time keeping
  rtcStamp->day = oneHex2twoDec(calendar->day);
  rtcStamp->month = oneHex2twoDec(calendar->month);
  yearMod100 = calendar->year % 100;
  rtcStamp->yearMod100 = oneHex2twoDec(yearMod100);
}

// used when setting the time to RTC
void Rtc_fromTai(RtcStamp* rtcStamp, unsigned long taitime) {
  Calendar calendar;
  Calendar_fromTai(&calendar, taitime);
  Rtc_fromCalendar(rtcStamp, &calendar);
}
