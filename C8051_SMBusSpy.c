// code by ntu ael

#include <c8051F120.h>

void SMBus_setGeneralReceiver(char enable) {
  char SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = SMB0_PAGE;
  if (enable)
    SMB0ADR |= 1;
  else
    SMB0ADR &= 0xfe;
  SFRPAGE = SFRPAGE_SAVE;
}

// char SMBus_isGeneralReceiver() := SMB0ADR & 1;
