// code by ntu ael

#ifndef OBDH_PORT
#define OBDH_PORT

void Port_init();

//-----------------------------------------------------------------------------
// Port 4-7 Pin Definitions
//-----------------------------------------------------------------------------

//sbit EN_5_ADS   = P4^0; //ADS    push pull
//sbit EN_5_ACS   = P4^1; //ACS    push pull
//sbit EN_5_PAYL    = P4^2; //PAYL   push pull
//sbit EN_5_DEPL    = P4^3; //DEPL   push pull
//sbit EN_COMM    = P4^4; //Comm   open drain
//sbit EN_SDC_1   = P4^5; //Main SDC with MOSFET open drain
//sbit EN_SDC_2   = P4^6; //Redundacy SDC with Schottky Diode push pull
//sbit EN_3V3_PAYL  = P4^7; //3V3 PAYL push pull
//
//sbit BATT_CTRL    = P5^0;  //BATT Control open drain
//sbit PWM_SEL_1    = P5^1;  //PWM SEL 1 open drain
//sbit PWM_SEL_2    = P5^2;  //PWM SEL 2 push pull
//sbit AMUX_SEL_1   = P5^3;  //AMUX SEL 1 push pull
//sbit AMUX_SEL_2   = P5^4;  //AMUX SEL 2 push pull
//sbit AMUX_SEL_3   = P5^5;  //AMUX SEL 3 push pull
//sbit OBDH_RESET   = P5^6;  //OBDH_RESET_N open drain
//sbit OBDH_OFF_VCC = P5^7;  //OBDH OFF VCC push pull
//
//sbit OBDH_FAULT   = P6^0;  //OBDH_FAULT Input - open drain
//sbit FLT_5V_ADS   = P6^1;  //FLT_5V_ADS Input - open drain
//sbit FLT_5V_ACS   = P6^2;  //FLT_5V_ACS Input - open drain
//sbit FLT_5V_PAYL  = P6^3;  //FLT_5V_PAYL Input - open drain
//sbit FLT_5V_DEPL  = P6^4;  //FLT_5V_DEPL Input - open drain
//sbit FLT_COMM   = P6^5;  //FLT_COMM Input - open drain
//sbit PWRGD_1    = P6^6;  //PWRGD_1 Input - open drain
//sbit PWRGD_2    = P6^7;  //PWRGD_2 Input - open drain
//
//sbit LED_1      = P7^0;  //LED 1 output push pull
//sbit LED_2      = P7^1;  //LED 2 output push pull
//sbit TK_SW_1    = P7^2;  //TK SW1 input open drain
//sbit TK_SW_2    = P7^3;  //TK SW2 input open drain
//sbit P7_4     = P7^4;  //spare open drain
//sbit P7_5     = P7^5;  //spare open drain
//sbit P7_6     = P7^6;  //spare open drain
//sbit P7_7     = P7^7;  //spare open drain

#endif
