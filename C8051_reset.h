// code by ntu ael

#ifndef C8051_RESET_H
#define C8051_RESET_H

// page 182 of datasheet:
// SFR Definition 13.2. RSTSRC: Reset Source
// Bit4: SWRSF: Software Reset Force and Flag.
// Write: 0: No effect.
// 1: Forces an internal reset. RST pin is not effected.
#define C8051_INTERNAL_RESET   SFRPAGE = LEGACY_PAGE; RSTSRC |= 0x10;

// page 182 of datasheet
// Forces a Power-On Reset. RST is driven low.
#define C8051_POWER_ON_RESET   SFRPAGE = LEGACY_PAGE; RSTSRC |= 0x01;

#endif
