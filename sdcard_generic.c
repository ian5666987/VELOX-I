// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "C8051_SPI0.h"
#include "C8051_SPI0_int.h"
#include "C8051_delay.h"

#include "sdcard_generic.h"

#define SD_GO_IDLE_STATE            0
#define SD_SEND_OP_COND             1
//#define SD_SEND_IF_COND             8 //comment out so that we don't accidentally use them. But may be used for future improvement..
//#define SD_SEND_CSD                 9
//#define SD_STOP_TRANSMISSION        12
#define SD_SEND_STATUS              13
//#define SD_SET_BLOCK_LEN            16
#define SD_READ_SINGLE_BLOCK        17
//#define SD_READ_MULTIPLE_BLOCKS     18
#define SD_WRITE_SINGLE_BLOCK       24
//#define SD_WRITE_MULTIPLE_BLOCKS    25
//#define SD_ERASE_BLOCK_START_ADDR   32
//#define SD_ERASE_BLOCK_END_ADDR     33
//#define SD_ERASE_SELECTED_BLOCKS    38
//#define SD_HC_SEND_OP_COND          41 // ACMD
//#define SD_APP_CMD                  55
//#define SD_READ_OCR                 58
//#define SD_CRC_ON_OFF               59

#define SD_CARD_BUFFER_SIZE         512

#define SD_ASSERT                   0
#define SD_DEASSERT                 1

void SD_initConfig() {
  g_sdConfig.time.init = 1;
  g_sdConfig.time.rnw = 1;
  g_sdConfig.time.store = 150;
  g_sdConfig.retry.cmd = 5;
  g_sdConfig.retry.cmd_reply = 5;
  g_sdConfig.retry.store_reply = 100;
}

char SD_spiWriteByte(char value) {
  SPI0_putGetChars(&value, 1);
  return value;
}

unsigned char SD_spiReadByte() {
  unsigned char l_uc_spi_received_buffer = 0xff;
  SPI0_putGetChars(&l_uc_spi_received_buffer, 1);
  return l_uc_spi_received_buffer;
}

// if there is need to change the polynomial, use 'extern' on global variable (for velox-1: no need)
unsigned char sd_crc7_ply = 0x12; // default value without initialization is chosen to be 0x12

//Private function, only visible to SD data
unsigned char SD_crc7Rotate(unsigned char crc, unsigned char nxt_byte) {
  unsigned char i, highest_bit;
  for (i = 0; i < 8; ++i) {
    highest_bit = crc >> 7;
    crc = (crc << 1) + ((nxt_byte >> (7 - i)) & 0x01);
    if (highest_bit)
      crc ^= sd_crc7_ply;
  }
  return crc;
}

unsigned char SD_crc7Update(void* msg, short length, unsigned char crc) {
  unsigned short i = 0;
  char* message = (char*) msg;
  for (i = 0; i < length; ++i)
    crc = SD_crc7Rotate(crc, message[i]);
  return crc;
}

unsigned char SD_crc7Publish(unsigned char crc) {
  crc = SD_crc7Rotate(crc, 0);
  crc |= 0x01;
  return crc;
}

char SD_getUntil(unsigned char response, unsigned char flag) {
  unsigned char value;
  unsigned char retry = 0;
  for (retry = 0; retry < g_sdConfig.retry.cmd_reply; ++retry) {
    value = SD_spiReadByte();
    if ((value & flag) == response)
      return 1;
    Delay_us(10);
  }
  return 0;
}

// return true if successful -> can trust value
// return false if limit reached -> do not trust content value
char SD_getUntilNot(unsigned char response, unsigned char flag, unsigned short delay_10us, unsigned char retry_limit, unsigned char* value_p) {
  unsigned char retry;
  unsigned short i;
  for (retry = 0; retry < retry_limit; ++retry) {
    *value_p = SD_spiReadByte();
    if ((*value_p & flag) != response)
      return 1;
    for (i = 0; i < delay_10us; ++i)
      Delay_us(10);
  }
  return 0;
}

//expected minimum time: 0.03 + 0.01 + 0.005 = 0.045ms
//expected maximum time: 0.3 + 0.3 + 0.05 = 0.65ms
char SD_sendCommand(char command, unsigned long arg, unsigned char* value_p, unsigned short delay_10us) {
  char success;
  char array[6];
  array[0] = command | 0x40;
  memcpy(&array[1], &arg, 4);
  array[5] = SD_crc7Publish(SD_crc7Update(array, 5, 0)); // there should be a checksum here
  NSSMD0 = SD_ASSERT;
  SPI0_putChars(array, 6); //Min-max time: 0.03 - 0.3ms
  success = SD_getUntilNot(0xff, 0xff, delay_10us, g_sdConfig.retry.cmd_reply, value_p); // we do not expect to get 0xff return here, but other return is passed to the caller
  SD_spiReadByte(); //Min-max time: 0.005 - 0.05ms. Extra 8 clock for buffering, due to hardware synchronization problem.Some SD cards will return two extra bytes, others are one byte
  NSSMD0 = SD_DEASSERT;

  return success;
}

char SD_commandUntil(char cmd, unsigned long arg, char response, unsigned short delay_10us) {
  char success;
  unsigned char value;
  unsigned char retry;
  for (retry = 0; retry < g_sdConfig.retry.cmd; ++retry) { //Works on the 1st trial most of the time. Set the number of retry to be 5 here.
    success = SD_sendCommand(cmd, arg, &value, delay_10us);
    if (success && value == response)
      return 1;
    Delay_us(10); //ian (2013-04-02): 10us works really well, gives about 10us delay here
  }
  return 0;
}

void SD_writeBlockData(void* buffer, short length) {
  short count;
  SD_spiWriteByte(0xfe); // Send start block token 0xfe (0x11111110) //Min-max time: 0.005 - 0.05ms
  SPI0int_process(SPI_put, buffer, length);
  for (count = length; count < SD_CARD_BUFFER_SIZE; ++count)
    SD_spiWriteByte(0x00); // Min-max time: 2.56 - 25.6ms
  SD_spiWriteByte(0xff); // transmit dummy CRC (16-bit), CRC is ignored here
  SD_spiWriteByte(0xff); // Min-max time: 0.01 - 0.1ms
}

void SD_readBlockData(void* buffer, short length) {
  short count;
  memset(buffer, 0xff, length);
  SPI0int_process(SPI_putGet, buffer, length);
  for (count = length; count < SD_CARD_BUFFER_SIZE; ++count)
    SD_spiReadByte(); // Min-max time: 2.56 - 25.6ms
  SD_spiReadByte(); // receive incoming CRC (16-bit), CRC is ignored here
  SD_spiReadByte();
  SD_spiReadByte(); // Extra 8 clock for buffering. Min-max time: 0.015 - 0.15ms
}

// expected minimum time: 0.045 + 0.1 + 0.01 + 0.095 = 0.16ms
// expected maximum time: 0.95 + 6 + 0.1 + 3.5 = 10.55ms
// experimental value: first time = 2 ms. Subsequent time < 1 ms
// ian (2013-04-03): 1 ms x 1000 x 2 = 2 seconds, giving SD card some time to start.
// Actual delay is 57% of the specified delay. (20s finished in 11.4s)
// with 24.5MHz system clock, it is expected to be 90%. (20s finished in 18s)
// with 22.1184MHz system clock, it is expected to be 99.75% (20s finished in 19.95s)
// with adjustment, 30s finished in 28.6s.
char SD_init() {
  unsigned char success = 1;
  unsigned char dummy_value;
  char array[10];
  memset(array, 0xff, sizeof array);
  if (!SD_sendCommand(SD_SEND_STATUS, 0, &dummy_value, 1)) { //Min-max time: 0.045-0.65ms ian (2013-05-29): there was a silly mistake here =.=' -> SD_sendCommand(SD_SEND_STATUS, 0) is changed to !SD_sendCommand(SD_SEND_STATUS, 0)
    SPI0_putChars(array, 10); // send 10 bytes of data to make sure that SD card is receiving. Min-max time: 0.05-0.5ms
    success = SD_commandUntil(SD_GO_IDLE_STATE, 0, 0x01, g_sdConfig.time.init); // ian (2013-07-22): first initialization time has to be set higher even 42ms is not enough for SANDisk SD card! Set as 70ms x 9 = 630ms for safety reason. Ian (2012-05-15): 0x01 is the expected return from SD card when it receives idle state command.
    if (!success)
      return 0;
    SPI0_putChars(array, 2); // Flushing, to make sure SD card is clear to receive command. Min-max time: 0.01 - 0.1ms
    success = SD_commandUntil(SD_SEND_OP_COND, 0, 0x00, 1);
  }
  return success;
}

// expected minimum time: 0.16 + 0.045 + 0.005 + 2.56 + 0.01 + 0.055 + 1.5 + 0.005 = 4.34 ms
// expected maximum time: 10.55 + 0.65 + 0.05 + 25.6 + 0.1 + 0.3 + 150 + 0.05 = 187.3 ms
// experimental value: long = 83 ms. short = 12 ms.
#define SD_CARD_COMMAND_SHIFT_BIT  9
char SD_writeSingleBlock(unsigned long block, void* buffer, short length) {
  unsigned char value;
  char success;
  if (!SD_init() || SD_CARD_BUFFER_SIZE < length) //ian (2013-04-09): add this. //Min-max time: 0.16 - 10.55ms
    return 0;
  success = SD_commandUntil(SD_WRITE_SINGLE_BLOCK, block << SD_CARD_COMMAND_SHIFT_BIT, 0, g_sdConfig.time.rnw); //ian 2013-07-22: 70us microsecond x 5 retry is minimum delay for SANDisk. Put 200us x 9 = 1.8ms for safety reason
  if (!success) // sdSendCommand shall return non-zero value when there is an error by hardware design.. check for SD status: 0x00 - OK (No flags set). // write a Block command. Min-max time: 0.045-0.65ms
    return 0;
  NSSMD0 = SD_ASSERT;
  SD_writeBlockData(buffer, length);
  if (!SD_getUntil(0x05, 0x1f)) //response= 0xXXX0AAA1; AAA='010' - data accepted, //AAA='101'-data rejected due to CRC error. //AAA='110'-data rejected due to write error
    return 0;
  if (!SD_getUntilNot(0x00, 0xff, g_sdConfig.time.store, g_sdConfig.retry.store_reply, &value)) // we are waiting for any non zero reply from SD card
    return 0;
  SD_spiWriteByte(0xff); // Extra 8 clock for buffering. Min-max time: 0.005 - 0.05ms
  NSSMD0 = SD_DEASSERT;
  return 1;
}

//expected minimum time: 0.16 + 0.045 + 0.055 + 2.56 + 0.015 = 2.835ms
//expected maximum time: 10.55 + 0.65 + 0.3 + 25.6 + 0.15 = 37.25ms
//experimental value: long = ?? ms. short = 11 ms. Always faster than writing!
char SD_readSingleBlock(unsigned long block, void* buffer, short length) {
  char success;
  if (!SD_init() || SD_CARD_BUFFER_SIZE < length) //ian (2013-04-09): add this. //Min-max time: 0.16 - 10.55ms
    return 0;
  success = SD_commandUntil(SD_READ_SINGLE_BLOCK, block << SD_CARD_COMMAND_SHIFT_BIT, 0, g_sdConfig.time.rnw); //ian 2013-07-22: 70us microsecond x 5 retry is minimum delay for SANDisk. Put 200us x 9 = 1.8ms for safety reason
  if (!success) // read a Block command. Min-max time: 0.045-0.65ms. check for SD status: 0x00 - OK (No flags set)
    return 0;
  NSSMD0 = SD_ASSERT;
  if (!SD_getUntil(0xfe, 0xff)) // 0xfe is the start block token expected to be received from SD card when it is ready to send one block of data.
    return 0;
  SD_readBlockData(buffer, length);
  NSSMD0 = SD_DEASSERT;
  return 1;
}
