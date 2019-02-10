// code by ntu ael

#ifndef RTC_GENERIC_H
#define RTC_GENERIC_H

#define twoDec2oneHex(a) ((a >> 4) * 10 + (a & 0x0F)) // 0x23 -> 23
#define oneHex2twoDec(a) (((a / 10) << 4) + (a % 10)) // 23 -> 0x23
// ---

struct struct_rtcStamp { // sizeof = 8
  unsigned char centis; // not in hex, but decimal(!); field is read only from RTC; when setting the RTC centis is 0
  unsigned char second;
  unsigned char minute;
  unsigned char hour;
  unsigned char weekday; // 1 ... 7
  unsigned char day; // 1 ... 31 warning
  unsigned char month;
  unsigned char yearMod100;
};

typedef struct struct_rtcStamp RtcStamp;

void Rtc_init(char* i2c_txbuffer, char* i2c_rxbuffer);
void Rtc_obtainStampPut();
char Rtc_obtainStampGet(RtcStamp* rtcStamp); // returns true or false
void Rtc_setRawTime(RtcStamp* rtcStamp);

#endif
