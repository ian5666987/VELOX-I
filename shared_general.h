// code by ntu ael

#ifndef SHARED_GENERAL_H
#define SHARED_GENERAL_H

// these addresses
// - 0xa0
// - 0xa2
// - 0xd0
// - 0x62
// - 0x64
// ...are reserved by 3rd party products

#define ADCS_ADDRESS 0x08
#define OBDH_ADDRESS 0x0a
#define PWRS_ADDRESS 0x0c
#define PSAT_ADDRESS 0x0e

#define CHECKSUM_PLY 0x8005

#define COMMAND_WATCHDOG_TEST    0x7f
#define WATCHDOG_TIMEOUT_MS     10000

enum enum_blobtype {
  // 0 is reserved
  blobtype_cam_thumb = 1, //
  blobtype_cam_full, // 2
  blobtype_cam_raw, // 3
  blobtype_cqt, // 4
  blobtype_gps_ascii, // 5
  blobtype_gps_binary, // 6
  blobtype_xbee_nsat // 7
};

// define the PSAT as either end device or router (BUT NOT BOTH!)
#define XBEE_END_DEVICE
//#define XBEE_ROUTER

// FRAME_LENGTH_MAX is not equivalent to ISIS comm board max frame length(!) but accounts for additional header info
#define FRAME_LENGTH_MAX  238  // value should be 238

#endif
