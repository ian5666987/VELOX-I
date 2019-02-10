// code by ntu ael

#include "Velox_runtime.h"
#include "Velox_uptime.h"

void Runtime_fromNow(Runtime* runtime) {
  runtime->from_clock = uptime.session;
}

char Runtime_hasExpired(Runtime* runtime) {
  return runtime->from_clock + (unsigned long) runtime->delay < uptime.session;
}
