// code by ntu ael

#ifndef MCCIF_SIGNAL_H
#define MCCIF_SIGNAL_H

// data structure is used to monitor external controls together with intrinsic variables
// mccifSignal is only set from the outside, and shall never be used in the logic of the satellite
//
// example application: when the satellite is on the turntable,
// the two angles of the table are also set as digital signals in the satellite
// so that the sensor values and control values can be monitored and logged together
struct struct_mccifSignal { // sizeof = 3 + 12 = 15
  char digital[3];
  float analog[3];
};

typedef struct struct_mccifSignal MccifSignal;

#endif
