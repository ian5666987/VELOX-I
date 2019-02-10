// code by ntu ael

#ifndef MCCIF_XDATA_H
#define MCCIF_XDATA_H

#include <c8051f120.h>

#define MCCIF_ID_CONTENT     0
#define MCCIF_ID_SET         1
#define MCCIF_ID_REQUEST     2

#define MCCIF_SYSTEM_OFFSET  5
#define MCCIF_OFFSET_OFFSET  6
#define MCCIF_LENGTH_OFFSET  8
#define MCCIF_DATA_OFFSET    9

#define MCCIF_COMMAND_MAXLEN 10

extern char mccif_port;
extern char mccif_localSystem;

extern char code mccif_xdatMarker[4];

void Mccif_setCompleted_Override(short offset);
short Mccif_formatContent(short offset, short length, char* message);
char Mccif_localSet(char* message);
short Mccif_requestContent(char* message, char* buffer);

#endif
