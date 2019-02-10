// code by ntu ael

#ifndef MCCIF_ACTIVE_H
#define MCCIF_ACTIVE_H

short Mccif_formatSet(char system, short offset, short length, void* value, char* message);
short Mccif_formatRequest(char system, short offset, short length, char* message);

#endif
