// code by ntu ael

#include <c8051F120.h>
#include <math.h>
#include <string.h>

#include "Velox_macros.h"

#define BEACON_NUMEL  16
#define BEACON_LAST   15

#if BEACON_LAST != BEACON_NUMEL - 1
#error "out of sync: beacon last"
#endif

// C, D, F, G, J, L, P, Q, Y, Z, 0, 2, 4, 6, 7, 8
// .  .  .  .           .     .     .  .  .     . // characters marked with . also exist on veloxp
// Important: the character E shall not be part of the beacon_alphabet since it's only a DOT, i.e. '.' and hard to isolate from noise!
char code beacon_alphabet[BEACON_NUMEL + 1] = "24678ACDFGJLPQYZ"; // +1 is required for 0 terminator, i.e. one extra char at the end
#define BEACON_UNASSIGNED_CHAR   '0'

void Beacon_setUnassigned(void* buffer, char length) {
  memset(buffer, BEACON_UNASSIGNED_CHAR, length);
}

char Beacon_identity(char value) {
  return beacon_alphabet[min(max(0, value), BEACON_LAST)]; // range check for safety
}

char Beacon_linear(float lo, float value, float hi) {
  float interp;
  if (hi <= value)
    return beacon_alphabet[BEACON_LAST];
  interp = (value - lo) / ((hi - 1) - lo);
  interp = min(max(0.0, interp), 1.0);
  return beacon_alphabet[(char) ceil(interp * (BEACON_LAST - 1))];
}
