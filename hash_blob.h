// code by ntu ael

#ifndef HASH_BLOB_H
#define HASH_BLOB_H

struct struct_hashBlob { // 4 + 1 = 5
  unsigned long offset;
  char copies;
};

extern struct struct_hashBlob g_hashBlob;

char Hash_putBlob(void* buffer, short length);
char Hash_getBlob(unsigned short index, char type, unsigned short count, void* buffer, short length);

#endif
