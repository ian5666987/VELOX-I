// code by ntu ael

//#include <c8051f120.h>
#include <string.h>

#include "Velox_calendar.h"

#define LEAP_SECONDS_UNTIL_PRESENT          35
#define SECONDS_PER_DAY                     86400L
#define SECONDS_PER_HOUR                    3600
#define SECONDS_PER_MINUTE                  60
#define MINUTES_PER_HOUR                    60
#define HOURS_PER_DAY                       24
// ---

unsigned char code calendar_limit_lo[SIZEOF_CALENDAR] = { 7, 0xA5, 12, 31, 23, 59, 25 }; // 1957 = 0x7A5 // 1957-12-31/23:59:25 == 0
unsigned char code calendar_limit_hi[SIZEOF_CALENDAR] = { 8, 0x2E, 02, 06, 06, 27, 40 }; // 2094 = 0x82E // 2094-02-06/06:27:40 == -1 == 0xffffffff

// private
void Calendar_initToTai0(Calendar* calendar) {
  memcpy(calendar, calendar_limit_lo, SIZEOF_CALENDAR);
}

// see Z:\datasheets\calibration\leapyear.m for test results
// year 1700, 1800, & 1900 are NOT leap years but year 1600 & 2000 are leap years, leap century occurred every 400 years
// private
char Calendar_isYearWith365Days(unsigned short year) { // ok
  return year % 4 || (!(year % 100) && (year % 400)); // common year is either not divisible 4 or divisible by 100 but not divisible by 400
}

unsigned char Calendar_daysOfMonth(unsigned short year, unsigned char month) { // ok
  if (month == month_february) // For month: Feb
    return Calendar_isYearWith365Days(year) ? 28 : 29;
  // Thirty days hath September, April, June, and November.
  // All the rest have thirty-one, Excepting February alone, ...
  return month == month_april || month == month_june || month == month_september || month == month_november ? 30 : 31;
}

// Calculate number of days between:
// a date and 1-JAN-1958, invalid to calculate days below the specified date
// private
unsigned short Calendar_daysSinceTaiUntil(unsigned short year, unsigned char month, unsigned char day) {
  unsigned short days;
  unsigned short yearCount;
  unsigned char monthCount;
  unsigned short yearStart;
  if (2013 <= year) {
    yearStart = 2013;
    days = 20089; // DAYS_FROM_TAI_TO_1_JAN_2013 == 20089
  } else {
    yearStart = 1958;
    days = 0;
  }
  for (yearCount = yearStart; yearCount < year; ++yearCount)
    days += Calendar_isYearWith365Days(yearCount) ? 365 : 366;
  for (monthCount = month_january; monthCount < month; ++monthCount)
    days += Calendar_daysOfMonth(year, monthCount);
  days += day - 1;
  return days;
}

void Calendar_incrementDay(Calendar* calendar) {
  ++calendar->day; //goto next day
  if (Calendar_daysOfMonth(calendar->year, calendar->month) < calendar->day) {
    calendar->day = 1; // goto next month
    ++calendar->month;
    if (month_december < calendar->month) {
      calendar->month = month_january; // goto next year
      ++calendar->year;
    }
  }
}

//06:23:00 6700 -> 18:23:54
// warning: seconds must not exceed SECONDS_PER_DAY
void Calendar_incrementSeconds(Calendar* calendar, unsigned long seconds) { // ok
  unsigned char hours;
  unsigned char minutes;
  hours = seconds / SECONDS_PER_HOUR;
  seconds %= SECONDS_PER_HOUR;
  minutes = seconds / SECONDS_PER_MINUTE;
  seconds %= SECONDS_PER_MINUTE;
  calendar->hour += hours;
  calendar->minute += minutes;
  calendar->second += seconds;
  if (SECONDS_PER_MINUTE <= calendar->second) {
    calendar->second -= SECONDS_PER_MINUTE;
    ++calendar->minute;
  }
  if (MINUTES_PER_HOUR <= calendar->minute) {
    calendar->minute -= MINUTES_PER_HOUR;
    ++calendar->hour;
  }
  if (HOURS_PER_DAY <= calendar->hour) {
    calendar->hour -= HOURS_PER_DAY;
    Calendar_incrementDay(calendar);
  }
}

void Calendar_setFromDays(Calendar* calendar, unsigned short days) { // ok
  unsigned short limit;
  do { // calculate year
    limit = Calendar_isYearWith365Days(calendar->year) ? 365 : 366;
    if (limit <= days) {
      days -= limit;
      ++calendar->year;
    } else
      break;
  } while (1);
  do { // calculate month
    limit = Calendar_daysOfMonth(calendar->year, calendar->month);
    if (limit <= days) {
      days -= limit;
      ++calendar->month;
    } else
      break;
  } while (1);
  calendar->day += days; // calculate days
}

// update dates from 31-DEC-1957 23:59:25
void Calendar_fromTai(Calendar* calendar, unsigned long taitime) {
  Calendar_initToTai0(calendar);
  if (LEAP_SECONDS_UNTIL_PRESENT <= taitime) { // if tai_time is greater than the no of seconds till end of the day, go to the next day
    taitime -= LEAP_SECONDS_UNTIL_PRESENT;
    Calendar_incrementDay(calendar);
    calendar->hour = 0;
    calendar->minute = 0;
    calendar->second = 0;
    Calendar_setFromDays(calendar, taitime / SECONDS_PER_DAY); // update day month and year
    taitime %= SECONDS_PER_DAY;
  }
  Calendar_incrementSeconds(calendar, taitime); // update the remaining seconds left
}

//Get no of second from 31-DEC-1957 23:59:25 by just inputing current date info (input range: 31-Dec-1957 23:59:25 to )
unsigned long Calendar_toTai(Calendar* calendar) {
  unsigned long taitime;
  if (memcmp(calendar, calendar_limit_lo, SIZEOF_CALENDAR) < 0) // below lower limit
    return 0;
  if (memcmp(calendar_limit_hi, calendar, SIZEOF_CALENDAR) < 0) // above upper limit
    return -1;

  if (calendar->year == 1957)
    return calendar->second - (60 - LEAP_SECONDS_UNTIL_PRESENT);

  taitime = LEAP_SECONDS_UNTIL_PRESENT;
  taitime += Calendar_daysSinceTaiUntil(calendar->year, calendar->month, calendar->day) * SECONDS_PER_DAY;
  taitime += calendar->hour * (unsigned long) SECONDS_PER_HOUR;
  taitime += calendar->minute * SECONDS_PER_MINUTE;
  taitime += calendar->second;
  return taitime;
}
