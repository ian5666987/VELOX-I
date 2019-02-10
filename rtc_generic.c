// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "C8051_SMBus.h"

#include "Velox_delay.h"

#include "rtc_generic.h"

#define RTC_SHORT_DELAY_MS  2 // jan changed to 2-3 ms
// ---
#define RTC_ADDRESS         0xD0 // RTC M41781S address -> 1101 0000
#define RTC_I2C_WR_LENGTH   21 // TODO page in data sheet
#define RTC_I2C_RD_LENGTH   20 // read length is less one than write because it does not need to use one byte to point to the initial position
// ---

char* rtc_txbuffer;
char* rtc_rxbuffer;

// This RTC library may need to come together with date and time library?
void Rtc_init(char* i2c_txbuffer, char* i2c_rxbuffer) {
  rtc_txbuffer = i2c_txbuffer;
  rtc_rxbuffer = i2c_rxbuffer;
}

// This function is to update RTC value to RTC chip literally
void Rtc_setRawTime(RtcStamp* rtcStamp) {
  memset(rtc_txbuffer, 0, RTC_I2C_WR_LENGTH);
  memcpy(&rtc_txbuffer[1], rtcStamp, sizeof(RtcStamp)); // TODO page in datasheet, comment what info needs to be passed in what bytes
  // ---
  SMBus_setTxBuffer(rtc_txbuffer, RTC_I2C_WR_LENGTH);
  SMBus_start(SMB_writeTo, RTC_ADDRESS);
  Delay_blockMs(RTC_SHORT_DELAY_MS);
}

// These functions are to obtain RTC value from the RTC chip literally
void Rtc_obtainStampPut() {
  SMBus_setRxBuffer(rtc_rxbuffer, RTC_I2C_RD_LENGTH);
  SMBus_start(SMB_readFrom, RTC_ADDRESS);
}

char Rtc_isConsistent(RtcStamp* rtcStamp) {
  char success;
  unsigned char value;
  success = 1;
  success &= rtcStamp->centis < 100;
  success &= twoDec2oneHex(rtcStamp->second) < 60;
  success &= twoDec2oneHex(rtcStamp->minute) < 60;
  success &= twoDec2oneHex(rtcStamp->hour) < 24;
  // ---
  value = twoDec2oneHex(rtcStamp->day);
  success &= 0 < value && value <= 31;
  value = twoDec2oneHex(rtcStamp->month);
  success &= 0 < value && value <= 12;
  success &= twoDec2oneHex(rtcStamp->yearMod100) < 100;
  return success;
}

// returns success status: true or false
// if successful the content of rtc_p contains the copy of the rtc memory with date info
char Rtc_obtainStampGet(RtcStamp* rtcStamp) { // sizeof RtcStamp is of different size from length
  short length;
  if (SMBus_isRxComplete(&length))
    if (length == RTC_I2C_RD_LENGTH) { // check length for expected length to have some notion of guarantee
      memcpy(rtcStamp, rtc_rxbuffer, sizeof(RtcStamp)); // get from milliseconds
      rtcStamp->centis = twoDec2oneHex(rtcStamp->centis); // convert to decimal, since much easier to post-process!
      return Rtc_isConsistent(rtcStamp);
    }
  return 0;
}
