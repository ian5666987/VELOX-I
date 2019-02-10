// code by ntu ael

#ifndef MCCIF_LOCAL_H
#define MCCIF_LOCAL_H

char Mccif_localHandle_Override(char* message);

void Mccif_localInit(char port, char address);
void Mccif_localSend(short offset, short length, void* buffer);
char Mccif_localPoll();

#endif
