// code by ntu ael

#include <c8051f120.h>

#include "sdcard_redundant.h"

#include "mccif_spy.h"

#include "hash_blob.h"

#include "shared_general.h"

#define   HASH_NIRVANA           0L

#define   HASH_CAMERA_MOD       16    // capacity is for 16 camera images (instead of 15 previously)
#define   HASH_CAMERA_SIZE    1000

#define   HASH_GPS_MOD           4    // capacity is for 4 gps log, separate for ascii and binary (total 8)
#define   HASH_GPS_SIZE        300

unsigned short code local_blobOffset[9] = { //
    0, // 0 ---> invalid
        10, // 1 thumb max 4000 B
        30, // 2 fullres max 200000 kB
        990, // 3 raw max 1542 B
        16000, // 4 cqt
        17400, // 5 gps asc (at most 297 frames) + 1200
        18600, // 6 gps bin (at most 297 frames) + 1200
        19800, // 7 xbee 2200 frames (for 36 min -> 1 frame per second)
        22000 }; // 8 <--- number to mark end of xbee domain

#if HASH_CAMERA_MOD * HASH_CAMERA_SIZE != 16000
#error "out of sync: cqt offset"
#endif

unsigned long Hash_addressBlob(unsigned short index, char type, unsigned short count) {
  unsigned long address;
  if (blobtype_cam_thumb <= type && type <= blobtype_xbee_nsat) { // check if valid type
    address = local_blobOffset[type] + count;
    if (address < local_blobOffset[type + 1]) { // address does not exceed boundary
      if (type <= blobtype_cam_raw) // if type is related to camera ...
        address += (index % HASH_CAMERA_MOD) * HASH_CAMERA_SIZE; // ... map into one of 16 segments
      else if (blobtype_gps_ascii <= type && type <= blobtype_gps_binary) // if type is related to gps ...
        address += (index % HASH_GPS_MOD) * HASH_GPS_SIZE; // ... map into one of 4 segments
      address *= g_hashBlob.copies;
      address += g_hashBlob.offset;
      return address;
    }
  }
  return HASH_NIRVANA;
}

// returns true only if address is valid
char Hash_putBlob(void* buffer, short length) {
  unsigned short index;
  char type;
  short count;
  unsigned long address;
  index = Mccif_getBlobIndex(buffer);
  type = Mccif_getBlobType(buffer);
  count = Mccif_getBlobCount(buffer);
  address = Hash_addressBlob(index, type, count);
  if (address != HASH_NIRVANA)
    return SDred_write(address, buffer, length, g_hashBlob.copies);
  return 0;
}

// returns true only if address is valid and content has correct checksum
char Hash_getBlob(unsigned short index, char type, unsigned short count, void* buffer, short length) {
  unsigned long address;
  char success;
  address = Hash_addressBlob(index, type, count);
  success = address != HASH_NIRVANA;
  if (success)
    success = SDred_read(address, buffer, length, g_hashBlob.copies);
  return success;
}
