// code by ntu ael

#include <c8051F120.h>
#include <intrins.h>   // include for _nop_ and other instructions
#include "C8051_oscillator.h"

// the file oscillator_config.h is required to be provided by the application.
// inside, the following constants need to be #defined:
// INT_OSC       // Internal oscillator frequency
// INT_PLL_MUL   // Set PLL multiplier N
// INT_PLL_DIV   // Set PLL divider M
// EXT_OSC       // External Crystal oscillator
// EXT_PLL_MUL   // Set PLL multiplier N
// EXT_PLL_DIV   // Sets PLL divider M

#include "oscillator_config.h"

struct struct_pll_settings {
	unsigned char flscl_flrt;
	unsigned char divider;
	unsigned char multiplier;
	unsigned char flt_loop;
	unsigned char flt_ico;
	long div_clk;
	long out_clk;
};

typedef struct struct_pll_settings PllSettings;

volatile OscType oscillator_type;
volatile unsigned long osc_sysclk;

char Oscillator_setXFCN (char* xfcn, long osc_speed){
	if (osc_speed <= 32000)
		*xfcn = 0;
	else if (osc_speed <= 84000L)
		*xfcn = 1;
	else if (osc_speed <= 225000L)
		*xfcn = 2;
	else if (osc_speed <= 590000L)
		*xfcn = 3;
	else if (osc_speed <= 1500000L)
		*xfcn = 4;
	else if (osc_speed <= 4000000L)
		*xfcn = 5;
	else if (osc_speed <= 10000000L)
		*xfcn = 6;
	else if (osc_speed <= 30000000L)
		*xfcn = 7;
	else
		return 0; //osc speed too high
	return 1;
}

char Oscillator_setPllDam (OscType osc_type, OscGear osc_gear, PllSettings* pll){
	char divider_set[10] 		= {4,4,1, 4,2,2,1, 4, 4,1}; //only for gear 4 and above, first 5 is ext divider, next 5 is int divider
	char multiplier_set[10] = {7,9,3,15,9,3,2,11,13,4}; //only for gear 4 and above, first 5 is ext multiplier, next 5 is int multiplier
	char choice_set;

	if (osc_gear < oscillator_gear4) //not valid for gear3 and below
		return 0;

	choice_set = osc_gear - oscillator_gear4 + osc_type * 5;
	pll->divider = divider_set[choice_set];
	pll->multiplier = multiplier_set[choice_set];

	return 1;
}

char Oscillator_prelimPllSetting (OscType osc_type, OscGear osc_gear, PllSettings* pll){
	long osc_speed;

	if (Oscillator_setPllDam(osc_type, osc_gear, pll)){ //if divider and multiplier can be set correctly, then proceeds, else return zero
		osc_speed = osc_type == ext_oscillator ? EXT_OSC : INT_OSC;
		pll->div_clk = osc_speed / pll->divider;
		pll->out_clk = pll->div_clk * pll->multiplier;

		if (pll->div_clk < 5000000L || pll->div_clk >= 30000001L || pll->out_clk < 25000000L || pll->out_clk >= 100000001L)
			return 0; //invalid result

		if (pll->div_clk < 8000000L)
			pll->flt_loop = 0x0f;
		else if (pll->div_clk < 12500000L)
			pll->flt_loop = 0x07;
		else if (pll->div_clk < 19500000L)
			pll->flt_loop = 0x03;
		else
			pll->flt_loop = 0x01;

		if (pll->out_clk < 42500000L)
			pll->flt_ico = 0x30;
		else if (pll->out_clk < 52500000L)
			pll->flt_ico = 0x20;
		else if (pll->out_clk < 72500000L)
			pll->flt_ico = 0x10;
		else
			pll->flt_ico = 0x00;

		pll->flscl_flrt = ((pll->out_clk-1)/25000000L) << 4; //from the if else statement below

//		if (pll->out_clk < 25000001L)
//			pll->flscl_flrt = 0x00;
//		else if (pll->out_clk < 50000001L)
//			pll->flscl_flrt = 0x10;
//		else if (pll->out_clk < 75000001L)
//			pll->flscl_flrt = 0x20;
//		else
//			pll->flscl_flrt = 0x30;
	} else
		return 0;

	return 1;
}

// Reset Source Register (RSTSRC) Refer page 182
// Overview:        This function will be executed to check for reset cause before initialize oscillator and set
//                  oscillator type to internal oscillator if reset cause is by missing clock, otherwise will be
//                  external oscillator.

OscType Oscillator_resetCauseCheck() {
  char SFRPAGE_SAVE = SFRPAGE;
  unsigned char l_uc_reset_source;
  OscType osc_type;
  SFRPAGE = LEGACY_PAGE;

  // Enable Missing Clock Detector
  // Select the VDD monitor as a reset source
  RSTSRC |= 0x06;

  // lsl : 28 Dec 2011 : 1742hrs : to determine the reset cause : begin

  // Set to external crystal initially
  osc_type = ext_oscillator;

  // Store RSTSRC
  l_uc_reset_source = RSTSRC;

  // if power on or hardware reset, set to external crystal
  if (!(l_uc_reset_source & 0x02)) {
//  if (!((l_uc_reset_source == 0x7F) || (l_uc_reset_source == 0xFF))) {
    if (l_uc_reset_source & 0x04) { // missing clock
      // if missing clock detected, set to internal crystal
      osc_type = int_oscillator;
    }
  }
  SFRPAGE = SFRPAGE_SAVE;
  return osc_type;
}

char Oscillator_setSysclkFromPll(OscType osc_type, OscGear osc_gear, PllSettings* pll) { //failure is not an option here
	char SFRPAGE_SAVE = SFRPAGE;
  short count;

  if (!Oscillator_prelimPllSetting (osc_type, osc_gear, pll)) //if the initialization is unsuccessful, returns 0
  	return 0;

  SFRPAGE = CONFIG_PAGE;

	// Derive SYSCLK (0x01 -> from external oscillator, 0x00 -> from internal oscillator)
	CLKSEL = !osc_type; //if osc_type = external, then this value is 1, zero otherwise

	// Set PLL source (0x04 -> from external oscillator, 0x00 -> from internal oscillator)
	PLL0CN = (!osc_type) << 2;

	if (osc_type == ext_oscillator)
		// Cache Control Register (CCH0CN) Refer to page 215
		CCH0CN &= ~0x20;

	SFRPAGE = LEGACY_PAGE;

	FLSCL = pll->flscl_flrt;

	SFRPAGE = CONFIG_PAGE;

	if (osc_type == ext_oscillator)
		CCH0CN |= 0x20;

  // Enable Power to PLL
  PLL0CN |= 0x01;

  // Set PLL divider value
  PLL0DIV = pll->divider;

  // Set the PLL filter loop and ICO bits
  PLL0FLT = pll->flt_loop; // Program the PLLLP3–0 bits (PLL0FLT.3–0) to the appropriate range for the divided reference frequency.
  PLL0FLT |= pll->flt_ico; // Program the PLLICO1-0 bits (PLL0FLT.5–4) to the appropriate range for the PLL output frequency.

  // Set PLL multiplier value
  PLL0MUL = pll->multiplier;

  // Wait at least 5us
  for (count = 0; count < 100; ++count)
    ;

  // Enable the PLL
  PLL0CN |= 0x02;

  // Wait until PLL frequency is locked
  while (!(PLL0CN & 0x10))
    ;

  // Select PLL as SYSCLK source
  CLKSEL = 0x02;

	SFRPAGE = SFRPAGE_SAVE;
	return 1;
}

// PreCondition:    only when oscillator_type is ext_oscillator
// Output:          return enum int_oscillator if clock is unstable, otherwise return enum ext_oscillator
// Overview:        This function will be executed to initialize external oscillator and set Phase-Locked Loop (PLL) to
//                  achieve frequency of 38707200 Hz.
OscType Oscillator_attemptExtInit(OscGear* osc_gear, PllSettings* pll) {
  char SFRPAGE_SAVE = SFRPAGE;
  char isStable;
  unsigned short count;
  char xfcn;

  if (*osc_gear < oscillator_gear3 || !Oscillator_setXFCN(&xfcn, EXT_OSC)) //External clock cannot go below gear3
  	return int_oscillator; //if xfcn initialization is failed, then returns internal oscillator

  SFRPAGE = CONFIG_PAGE;

  // External Oscillator Control Register (OSCXCN) Refer page 189
  OSCXCN = 0x60;
  OSCXCN |= xfcn;

  // Wait 1ms for initialization
  for (count = 0; count < 25000; count++) {
    _nop_();
  }

  // XTLVLD = 1 -> Crystal Oscillator is running and stable
  for (count = 0; count < 3000; ++count) {
    isStable = OSCXCN & 0x80;
    if (isStable)
      break;
  }

	if (isStable) {
		if (*osc_gear >= oscillator_gear4) {  //attempt to use PLL for high gear only, don't run the function for lower gear!!!
			if (!Oscillator_setSysclkFromPll(ext_oscillator, *osc_gear, pll)) { //if unsuccessful, then use normal system clock
				*osc_gear = oscillator_gear3; //set to gear3
				CLKSEL = 0x01; //SYSCLK is derived from External oscillator circuit without PLL
			}
		} else
			CLKSEL = 0x01; //SYSCLK is derived from External oscillator circuit without PLL
	} else {
		SFRPAGE = SFRPAGE_SAVE;
		return int_oscillator;
	}

	SFRPAGE = SFRPAGE_SAVE;
	return ext_oscillator;
}

// PreCondition:    only when oscillator_type is int_oscillator
// Overview:        This function will be executed to initialize internal oscillator and set Phase-Locked Loop (PLL) to
//                  achieve frequency of 36750000 Hz.
void Int_Oscillator_Init(OscGear* osc_gear, PllSettings* pll) {
	char SFRPAGE_SAVE = SFRPAGE;
	SFRPAGE = CONFIG_PAGE;

	if (*osc_gear >= oscillator_gear4) {  //attempt to use PLL for high gear only, don't run the function for lower gear!!!
		//Set internal oscillator to run at its maximum frequency
		OSCICN = 0x83;
		if (!Oscillator_setSysclkFromPll(int_oscillator, *osc_gear, pll)) { //if unsuccessful, then use normal system clock
			*osc_gear = 3; //Reset to gear 3 when unsuccessful
			OSCICN = 0x83;
			CLKSEL = 0x00; //SYSCLK is derived from internal oscillator circuit without PLL
		}
	} else {
		OSCICN = 0x80 + *osc_gear; //To divide internal oscillator by 1, 2, 4, or 8
		CLKSEL = 0x00; //To use internal oscillator as reference for system clock
	}

	SFRPAGE = SFRPAGE_SAVE;
}

// PreCondition:    only when oscillator_type is ext_oscillator
// Overview:        This function will be executed in the main function to determine which type of oscillator to be
//                  used and initialize the oscillator to achieve desired SYSCLK frequency.
void Oscillator_init(OscGear osc_gear) { //ian 20130718: different gear to control the oscillator speed
	PllSettings pll;
	if (osc_gear > oscillator_gear8)
		osc_gear = oscillator_gear8; //Limit gear to gear8
  oscillator_type = Oscillator_resetCauseCheck();
  if (oscillator_type == ext_oscillator)
    oscillator_type = Oscillator_attemptExtInit(&osc_gear, &pll);
  // ---
  if (oscillator_type == int_oscillator)
    Int_Oscillator_Init(&osc_gear, &pll);
  // --- calculate system clock based on the latest gear applied
	if (osc_gear >= oscillator_gear4) //for gear 4 and above, use PLL output clock calculation
		osc_sysclk = pll.out_clk;
	else if (osc_gear == oscillator_gear3 && oscillator_type == ext_oscillator) //there are two cases
		osc_sysclk = EXT_OSC;
	else //for gear3 and lower, other than the case above, it must use internal clock
		osc_sysclk = INT_OSC / (8 >> osc_gear); //gear3 = INT_OSC/1, gear2 = INT_OSC/2, gear1 = INT_OSC/4, gear0 = INT_OSC/8,
}

// Output:          return oscillator type (0 is external oscillator, 1 is internal oscillator)
// Overview:        This function will be executed get the type of oscillator used.
OscType Oscillator_getType() {
  return oscillator_type;
}

// Output:          return value of system clock frequency
// Overview:        This function will be executed get the SYSCLK frequency based on oscillator type.
long Oscillator_getSystemClock() {
	return osc_sysclk;
}
