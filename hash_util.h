// code by ntu ael

#ifndef HASH_UTIL_H
#define HASH_UTIL_H

#include "Velox_slowtask.h"

#include "hash_frame.h"

#define DOWNLINK_IGNORE_LO     0xff
#define DOWNLINK_IGNORE_HI     0xffff

#define GETBLOB_LO_NUMEL       10
#define GETBLOB_HI_NUMEL        5

struct struct_getBlob { // 1 + 1 + 2 + 2 + 10 + 10 = 26
  char index;
  char type;
  unsigned short head;
  unsigned short tail;
  unsigned char lo[GETBLOB_LO_NUMEL];
  unsigned short hi[GETBLOB_HI_NUMEL];
};

extern struct struct_getBlob g_getBlob; //

enum enum_putIterator {
  putIterator_inactive = 0, //
  putIterator_fused, //
  putIterator_active,
};

struct struct_putFrame { // 3 + 5 + 1 + 4 = 13 ian: following the change on FrameIndex
  Slowtask task;
  enum enum_putIterator putIterator;
  FrameIndex frameIndex;
  unsigned long mask;
};

extern struct struct_putFrame g_putFrame;

struct struct_getFrame { // 5 + 4 + 1 + 2 = 12 ian: following the change on FrameIndex
  FrameIndex frameIndex;
  short numPages;
  unsigned long mask;
  unsigned char stride;
};

extern struct struct_getFrame g_getFrame;

struct struct_pinFrame { // 5 + 4 = 9 ian: following the change on FrameIndex
  FrameIndex frameIndex;
  unsigned long mask;
};

extern struct struct_pinFrame g_pinFrame;

struct struct_manifest { // TODO needs field for page
  unsigned long offset; // absolute first address
  short divider;
  char copies; // redundancy, typically greater than 1 for vital information!
  char failures;
};

extern struct struct_manifest g_manifest;

#endif
