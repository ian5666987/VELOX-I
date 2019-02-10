// code by ntu ael

#include <c8051f120.h>

#include "C8051_adc.h"
#include "C8051_delay.h"
#include "C8051_extern_int.h"
#include "C8051_mcutemp.h"
#include "C8051_oscillator.h"
#include "C8051_pca0.h"
#include "C8051_SMBus.h"
#include "C8051_SMBusSpy.h"
#include "C8051_SMBusTimer3.h"
#include "C8051_SPI0_shared.h"
#include "C8051_UART1Timer1.h"
#include "C8051_UART0Timer4.h"
#include "C8051_watchdog.h"

#include "Velox_meta.h"

#include "obdh_constant.h"
#include "obdh_timer.h"
#include "obdh_systemconfig.h"

#include "shared_general.h"

void SystemInit_chip(OscGear oscGear) {
  EA = 0;
  EA = 0; // this is a dummy instruction with two-byte opcode. do not remove

  Oscillator_init(oscGear); //Initialize with gear4
  Meta_init(); // required for chip_meta.systemFreq

  // --- c8051
  SPI0_init(chip_meta.systemFreq, F_SCK_MAX, SPI0_CONFIG, SPI0_TIMEOUT);
  Timer0_init(chip_meta.systemFreq, System_getCKCON(oscGear)); // used for watchdog and various delays
  UART1Timer1_init(chip_meta.systemFreq, System_getT1Prescaler(oscGear), BAUDRATE_1); // used for UART1 baudrate generator
  Timer2_init(chip_meta.systemFreq); // not used, but initialized to prevent floating state
  UART0Timer4_init(chip_meta.systemFreq, BAUDRATE_0); // used for UART0 baudrate generator
  ADC0_init(chip_meta.systemFreq, VREF_MV);
  SMBus_init(OBDH_ADDRESS, chip_meta.systemFreq);
  SMBusTimer3_init(chip_meta.systemFreq); // Initialize Timer 3: used for I2C SCL timeout (SMBus)

  EA = 1;
}
