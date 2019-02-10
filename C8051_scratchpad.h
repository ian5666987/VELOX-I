// Copyright 2004 Silicon Laboratories, Inc.
// This program contains several useful utilities for writing and updating
// FLASH memory.
// AUTH: BW & GP
// DATE: 21 JUL 04
// Target: C8051F12x
// Tool chain: KEIL C51 7.06
// Release 1.1
// -Change typecast of FLASH_PAGESIZE to 1024L to fix bug in Flash_Clear()
// -07 FEB 2006 (GP)
// Release 1.0

#ifndef SCRATCHPAD_BYTE_H
#define SCRATCHPAD_BYTE_H

typedef unsigned long FLADDR;

// FLASH read/write/erase routines
extern void FLASH_ByteWrite(FLADDR addr, char byte, bit SFLE);
extern unsigned char FLASH_ByteRead(FLADDR addr, bit SFLE);
extern void FLASH_PageErase(FLADDR addr, bit SFLE);

#endif
