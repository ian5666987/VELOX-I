// code by ntu ael

#ifndef VELOX_ORBIT_H
#define VELOX_ORBIT_H

enum enum_orbitStatus {
  orbit_statusUnknownOrNoChange, //
  orbit_eclipse, //
  orbit_sunlight, //
};

struct struct_orbit { // 4 + 2 + 2
  unsigned long eclipseEntry; // taitime UTC
  short eclipseDuration; // sec
  short period; // sec, short is sufficient, since 100 minutes are 6000 sec
};

extern struct struct_orbit g_orbit;

void Orbit_initNull();
//char Orbit_isValid();
enum enum_orbitStatus Orbit_getStatusNow();
enum enum_orbitStatus Orbit_getTransitionChange();

#endif
