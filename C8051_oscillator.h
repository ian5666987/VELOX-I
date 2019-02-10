// code by ntu ael

#ifndef C8051_OSCILLATOR_H
#define C8051_OSCILLATOR_H

enum enum_oscType {
  ext_oscillator = 0, //
  int_oscillator
};

typedef enum enum_oscType OscType;

enum enum_oscGear {
  oscillator_gear0 = 0, //            | int:  3,062,500
  oscillator_gear1, //                | int:  6,125,000
  oscillator_gear2, //                | int: 12,250,000
  oscillator_gear3, //ext: 22,118,400 | int: 24,500,000
  oscillator_gear4, //ext: 38,707,200 | int: 36,750,000 | use PLL | ext:  7/4 | int:  3/2
  oscillator_gear5, //ext: 49,766,400 | int: 49,000,000 | use PLL | ext:  9/4 | int:  2/1
  oscillator_gear6, //ext: 66,355,200 | int: 67,375,000 | use PLL | ext:  3/1 | int: 11/4
  oscillator_gear7, //ext: 82,944,000 | int: 79,625,000 | use PLL | ext: 15/4 | int: 13/4 //divider cannot exceed 4
  oscillator_gear8, //ext: 99,532,800 | int: 98,000,000 | use PLL | ext:  9/2 | int:  4/1
};

typedef enum enum_oscGear OscGear;

void Oscillator_init(OscGear osc_gear);
OscType Oscillator_getType();
long Oscillator_getSystemClock();

#endif
