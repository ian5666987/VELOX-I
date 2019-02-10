// code by ntu ael

#ifndef VELOX_BEACON_H
#define VELOX_BEACON_H

void Beacon_setUnassigned(void* buffer, char length);
char Beacon_identity(char value);
char Beacon_linear(float lo, float value, float hi);

#endif
