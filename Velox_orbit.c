// code by ntu ael

#include <string.h>

#include "Velox_orbit.h"
#include "Velox_uptime.h"

// call before entering main loop
void Orbit_initNull() {
  memset(&g_orbit, 0, sizeof g_orbit);
}

char Orbit_isValid() {
  char success = 1;
  success &= Uptime_isTaiTimeValid();
  success &= g_orbit.eclipseEntry != 0L;
  success &= 0 <= g_orbit.eclipseDuration; // <= is deliberate to allow to cheat the satellite in believing there is always sunlight
  success &= g_orbit.eclipseDuration < g_orbit.period; // also ensures that 0 < period
  return success;
}

short Orbit_getPhase() {
  long delta;
  delta = uptime.tai_time - g_orbit.eclipseEntry;
  delta %= g_orbit.period;
  if (delta < 0)
    delta += g_orbit.period;
  return delta;
}

enum enum_orbitStatus Orbit_getStatusNow() {
  if (Orbit_isValid())
    return Orbit_getPhase() < g_orbit.eclipseDuration ? orbit_eclipse : orbit_sunlight;
  return orbit_statusUnknownOrNoChange;
}

// has to be called at least every time uptime changes, i.e. every second, or in combination with Seconds_haveAdvanced()
enum enum_orbitStatus Orbit_getTransitionChange() {
  short delta;
  if (Orbit_isValid()) {
    delta = Orbit_getPhase();
    if (delta == 0)
      return orbit_eclipse;
    if (delta == g_orbit.eclipseDuration)
      return orbit_sunlight;
  }
  return orbit_statusUnknownOrNoChange;
}
