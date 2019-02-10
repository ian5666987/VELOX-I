// code by ntu ael

#ifndef OBDH_RECON_H
#define OBDH_RECON_H

char Subsystem_isAvailable(char address);
char SDCard_isAvailable();
void Recon_ChannelStatusUpdated_Event();
char Recon_isDebugPrintEnabled();
void Recon_debugPrint(void* buffer, short length);


#endif
