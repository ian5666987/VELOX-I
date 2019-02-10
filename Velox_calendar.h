// code by ntu ael

#ifndef VELOX_CALENDAR_H
#define VELOX_CALENDAR_H

#include <c8051f120.h>

#define SIZEOF_CALENDAR   7

struct struct_calendar { // sizeof = 2 + 5 = 7
  unsigned short year; // for instance 2013
  unsigned char month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
};

typedef struct struct_calendar Calendar;

enum enum_month {
  month_january = 1, //
  month_february, //
  month_march, //
  month_april, //
  month_may, //
  month_june, //
  month_july, //
  month_august, //
  month_september, //
  month_october, //
  month_november, //
  month_december //
};

extern unsigned char code calendar_limit_lo[SIZEOF_CALENDAR];

unsigned char Calendar_daysOfMonth(unsigned short year, unsigned char month);

unsigned long Calendar_toTai(Calendar* calendar);
void Calendar_fromTai(Calendar* calendar, unsigned long taitime);

#endif
