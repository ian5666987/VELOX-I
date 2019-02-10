// code by ntu ael

#ifndef VELOX_SFRSTACK_H
#define VELOX_SFRSTACK_H

#define SFRSTACK_UNDERRUN   0x40
#define SFRSTACK_OVERRUN    0x80

void SFR_init(char* stack, short stack_size);
void SFR_pushPage(char page);
void SFR_popPage();
char SFR_getMaxDepth();

#endif
