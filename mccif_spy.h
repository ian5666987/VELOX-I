// code by ntu ael

#ifndef MCCIF_SPY_H
#define MCCIF_SPY_H

short Mccif_getArgLength(void* buffer);
short Mccif_getFrameLength(void* buffer);
unsigned short Mccif_getBlobIndex(void* buffer);
char Mccif_getBlobType(void* buffer);
unsigned short Mccif_getBlobCount(void* buffer);
char Mccif_isValidContent(void* buffer, short max_length);

#endif
