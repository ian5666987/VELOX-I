// code by ntu ael

#ifndef VELOX_UPTIME_H
#define VELOX_UPTIME_H

#define DECLARE_UPTIME_GLOBALVARIABLES_AT(x) \
    struct struct_uptime xdata uptime _at_ (x); \

// eclipse needs extra backslash. keep the white space in the line above!

#define TAI_TIME_UNAVAILABLE 0L
#define UPTIME_PERSISTENT_LENGTH 10 // 4 + 4 + 2 = 10

struct struct_uptime { // 4 + 4 + 2 + 4 = 14
  unsigned long tai_time; // seconds since 1957-01-01/23:59:25 UTC (the 35 seconds until new year account for the leap seconds)
  unsigned long total; // estimated number of seconds of on time of the chip since mission begin (preserve this value periodically in Eeprom/sd card using void Uptime_storeVitals_Override())
  unsigned short storePeriod; // unit = seconds
  unsigned long session; // number of seconds since last reset
};

extern struct struct_uptime uptime;

void Uptime_init(unsigned long tai_time, unsigned long uptime_total, unsigned short period);
char Uptime_isTaiTimeValid();
char Uptime_handler();

#endif
