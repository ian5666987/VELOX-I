// code by ntu ael

#ifndef VELOX_MACROS_H
#define VELOX_MACROS_H

#define min(x,y)  ( (x)<(y)?  (x) : (y))
#define max(x,y)  ( (x)<(y)?  (y) : (x))
#define signum(x) ( (x)< 0 ? (-1) : ( 0<(x) ? 1 : 0) )

#define extractFromBuffer(type, ptr) (*((type *)(ptr)))

#endif
