// code by ntu ael

#ifndef VELOX_RUNTIME_H
#define VELOX_RUNTIME_H

struct struct_runtime {
  unsigned long from_clock; // uptime session sec
  unsigned short delay; // sec
};

typedef struct struct_runtime Runtime;

void Runtime_fromNow(Runtime* runtime);
char Runtime_hasExpired(Runtime* runtime);

#endif
