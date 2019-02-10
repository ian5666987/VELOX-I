// code by ntu ael

#include <c8051f120.h>

#include "Velox_sfrstack.h"

#include "obdh_port.h"

sbit ON_SD = P4 ^ 1;
sbit OE_USB = P4 ^ 4;

#define ON_USB 0
#define SD_ASSERT    0  // redundant
#define SD_DEASSERT  1

void Port_init() {
  // P0.7  -  SCL (SMBus), Open-Drain, Digital
  // P0.6  -  SDA (SMBus), Open-Drain, Digital
  // P0.5  -  NSS  (SPI0), Open-Drain,  Digital
  // P0.4  -  MOSI (SPI0), Open-Drain,  Digital
  // P0.3  -  MISO (SPI0), Open-Drain, Digital
  // P0.2  -  SCK  (SPI0), Open-Drain,  Digital
  // P0.1  -  RX0 (UART0), Open-Drain, Digital
  // P0.0  -  TX0 (UART0), Push-Pull,  Digital

  // P1.7  -  CEX3 (PCA),  Push-Pull, Digital
  // P1.6  -  CEX2 (PCA),  Push-Pull,  Digital
  // P1.5  -  CEX1 (PCA),  Push-Pull,  Digital
  // P1.4  -  CEX0 (PCA),  Push-Pull,  Digital
  // P1.3  -  RX1 (UART1), Open-Drain, Digital
  // P1.2  -  TX1 (UART1), Push-Pull,  Digital
  // P1.1  -  AIN2.1,     Open-Drain, Analog
  // P1.0  -  AIN2.0,     Open-Drain, Analog

  // P2.7  -  T1 (Timer1), Open-Drain,  Digital
  // P2.6  -  INT0 (Tmr0), Open-Drain, Digital
  // P2.5  -  T0 (Timer0), Open-Drain,  Digital
  // P2.4  -  CP1 (Cmpr1), Open-Drain,  Digital
  // P2.3  -  CP0 (Cmpr0), Open-Drain,  Digital
  // P2.2  -  ECI,         Open-Drain, Digital
  // P2.1  -  CEX5 (PCA),  Push-Pull, Digital
  // P2.0  -  CEX4 (PCA),  Push-Pull, Digital

  // P3.7  -  CNVSTR2,     Open-Drain, Digital
  // P3.6  -  CNVSTR0,     Open-Drain, Digital
  // P3.5  -  SYSCLK,      Open-Drain,  Digital
  // P3.4  -  T4EX (Tmr4), Open-Drain, Digital
  // P3.3  -  T4 (Timer4), Open-Drain, Digital
  // P3.2  -  T2EX (Tmr2), Open-Drain, Digital
  // P3.1  -  T2 (Timer2), Open-Drain, Digital
  // P3.0  -  INT1 (Tmr1), Open-Drain, Digital

  //  EN_5_ADS    = P4^0; //ADS    push pull
  //  EN_5_ACS    = P4^1; //ACS    push pull
  //  EN_5_PAYL   = P4^2; //PAYL   push pull
  //  EN_5_DEPL   = P4^3; //DEPL   push pull
  //  EN_COMM     = P4^4; //Comm   open drain
  //  EN_SDC_1    = P4^5; //Main SDC with MOSFET open drain
  //  EN_SDC_2    = P4^6; //Redundacy SDC with Schottky Diode push pull
  //  EN_3V3_PAYL = P4^7; //3V3 PAYL push pull
  //
  //  BATT_CTRL    = P5^0;  //BATT Control open drain
  //  PWM_SEL_1    = P5^1;  //PWM SEL 1 open drain
  //  PWM_SEL_2    = P5^2;  //PWM SEL 2 push pull
  //  AMUX_SEL_1   = P5^3;  //AMUX SEL 1 push pull
  //  AMUX_SEL_2   = P5^4;  //AMUX SEL 2 push pull
  //  AMUX_SEL_3   = P5^5;  //AMUX SEL 3 push pull
  //  OBDH_RESET   = P5^6;  //OBDH_RESET_N input open drain
  //  OBDH_OFF_VCC = P5^7;  //OBDH OFF VCC push pull
  //
  //  OBDH_FAULT  = P6^0;  //OBDH_FAULT Input - open drain
  //  FLT_5V_ADS  = P6^1;  //FLT_5V_ADS Input - open drain
  //  FLT_5V_ACS  = P6^2;  //FLT_5V_ACS Input - open drain
  //  FLT_5V_PAYL = P6^3;  //FLT_5V_PAYL Input - open drain
  //  FLT_5V_DEPL = P6^4;  //FLT_5V_DEPL Input - open drain
  //  FLT_COMM    = P6^5;  //FLT_COMM Input - open drain
  //  PWRGD_1     = P6^6;  //PWRGD_1 Input - open drain
  //  PWRGD_2     = P6^7;  //PWRGD_2 Input - open drain
  //
  //  LED_1     = P7^0;  //LED 1 output push pull
  //  LED_2     = P7^1;  //LED 2 output push pull
  //  TK_SW_1   = P7^2;  //TK SW1 input open drain
  //  TK_SW_2   = P7^3;  //TK SW2 input open drain
  //  P7_4      = P7^4;  //spare open drain
  //  P7_5      = P7^5;  //spare open drain
  //  P7_6      = P7^6;  //spare open drain
  //  P7_7      = P7^7;  //spare open drain

  // crossbar enabled, disable weak push-ups

  SFR_pushPage(CONFIG_PAGE); //Save current SFR page and set SFR page
  // PWRS config
  //  XBR0 = 0xF7;
  //  XBR1 = 0xFF;
  //  XBR2 = 0x7D;
  //  P1MDIN = 0xFC;
  //  P1|=0x03; // based on page 240, data bits need to set to 1 for analog pins
  //  P0MDOUT = 0x01;
  //  P1MDOUT = 0xF4;
  //  P2MDOUT = 0x03;
  //  P3MDOUT = 0x00;
  //  P4MDOUT = 0xCF;
  //  P5MDOUT = 0xBC;
  //  P6MDOUT = 0x00;
  //  P7MDOUT = 0x03;

  //  char SFRPAGE_SAVE = SFRPAGE;
  //
  //  SFRPAGE = CONFIG_PAGE;

  XBR0 |= 0x07; //To enable UART0 (XBR0^2), SPI (XBR0^1), and SM Bus (XBR0^0)

  P0MDOUT &= ~0xC0; //P0^6-7 pins open-drain output (that SDA and SCL lines
  //may draw current from outsource power supply)

  //To make P0^2 to P0^5 (SPI pins) push-pull pins too
  P0MDOUT |= 0x3D; //Make UART0 TX(P0.0 push-pull) (that it may send data
  //through UART cable by its own power supply)
  P0 = 0xFF; //By default, all pins will be 'high'

  P1MDOUT = 0x00; //All P1 pins open-drain output (that UART1
  //may draw current from outsource power supply)
  P1MDOUT |= 0x01; //Make UART1 TX(P1.0 push-pull) (that it may send data
  //through UART cable by its own power supply)
  P1 = 0xFF; //By default, all pins will be 'high'

  P4MDOUT |= 0x12; //To make P4^1 & P4^4 pins (-ON_SD pin & -OE_USB) as push-pull
//  P4MDOUT |= 0x13;  //add P4^0 as a push-pull port because this P4^0 is going to be used as an indicator
//                    //for I2C lines down timeout. Originally, statement P4MDOUT |= 0x12; should be enough
//                    //In the hardware, this corresponds to H2.18 pin.
  P4 = 0xFF; //make sure that all pins in port 4 are enabled as high first.
  OE_USB = ON_USB; //Enable USB port

  // EMIF port setting;
  P4MDOUT |= 0xE0; // To make P4^5 (ALE), P4^6 (-READ), P4^7 (-WRITE) push-pull.
  P5MDOUT |= 0xC0; // To make P5^6(-MEMBUS), P5^7(MEMBANK) push pull;
  P6MDOUT |= 0xFF;
  P7MDOUT |= 0xFF; // To make data line, address line push-pull;

  ON_SD = SD_ASSERT; // Turn on SD Card
  NSSMD0 = SD_DEASSERT; //Disable chip select for now

  XBR2 |= 0x44; //To enable crossbar and weak pull-ups

  SFR_popPage(); //Restore SFR page
}
