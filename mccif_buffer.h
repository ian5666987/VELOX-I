// code by ntu ael

#ifndef MCCIF_BUFFER_H
#define MCCIF_BUFFER_H

void Mccif_setBuffer(void* buffer, short length);
void Mccif_flush();
void Mccif_appendChar(char index, char value);
void Mccif_appendChars(char index, void* ptr, short len);

#endif
