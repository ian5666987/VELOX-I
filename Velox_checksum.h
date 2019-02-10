// code by ntu ael

#ifndef VELOX_CHECKSUM_H
#define VELOX_CHECKSUM_H

void Checksum_init(unsigned short ply);
unsigned short Checksum_update(void* msg, short length, unsigned short crc);
unsigned short Checksum_publish(unsigned short crc);
char Checksum_isValid(void* msg, short length);

#endif
