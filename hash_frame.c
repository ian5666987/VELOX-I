// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "sdcard_redundant.h"

#include "mccif_spy.h"

#include "hash_frame.h"

// private function
unsigned long Hash_getAddress(FrameIndex* frameIndex) {
  unsigned long page = frameIndex->page; //ian: following the change on FrameIndex
  page %= g_hashFrame.pages; // safety measure to prevent out of bounds/"pages" access, however does not alter value of frameIndex->page (because no immediate need)
  return g_hashFrame.offset + (page * (long) g_hashFrame.stride + frameIndex->index) * (long) g_hashFrame.copies;
}

char Hash_getFrame(FrameIndex* frameIndex, void* buffer, short length) {
  memset(buffer, 0, length); // to assert that buffer does not contain relics of old frame data
  return SDred_read(Hash_getAddress(frameIndex), buffer, length, g_hashFrame.copies);
}

char Hash_putFrame(FrameIndex* frameIndex, void* buffer, short length) {
  return SDred_write(Hash_getAddress(frameIndex), buffer, length, g_hashFrame.copies);
}

//#define HASH_ADDRESS_NOT_ALLOCATED 0xFFFFFFFF
//
//// private function
//char Hash_getRelativeIndex(char absIndex, unsigned long mask){
//	char i, relativeIndex = -1;
//	for (i = 0; i < absIndex; i++)
//		if (mask & (1L << i))
//			relativeIndex++;
//	return relativeIndex;
//}
//
//unsigned long Hash_getAddress(FrameIndex* frameIndex, unsigned long mask) {
//  unsigned long page = frameIndex->page; //ian: following the change on FrameIndex
//  page %= g_hashFrame.pages; // safety measure to prevent out of bounds/"pages" access, however does not alter value of frameIndex->page (because no immediate need)
//  char relativeIndex = Hash_getRelativeIndex(frameIndex->index, mask);
//  return relativeIndex >= 0 ? g_hashFrame.offset + (page * (long) g_hashFrame.stride + relativeIndex) * (long) g_hashFrame.copies : HASH_ADDRESS_NOT_ALLOCATED;
//}
//
//char Hash_getFrame(FrameIndex* frameIndex, void* buffer, short length) {
//  memset(buffer, 0, length); // to assert that buffer does not contain relics of old frame data
//  unsigned long address = Hash_getAddress(frameIndex, g_getFrame.mask);
//  return address == HASH_ADDRESS_NOT_ALLOCATED ? 0 : SDred_read(address, buffer, length, g_hashFrame.copies);
//}
//
//char Hash_putFrame(FrameIndex* frameIndex, void* buffer, short length) {
//	unsigned long address = Hash_getAddress(frameIndex, g_putFrame.mask);
//  return address == HASH_ADDRESS_NOT_ALLOCATED ? 0 : SDred_write(address, buffer, length, g_hashFrame.copies);
//}
