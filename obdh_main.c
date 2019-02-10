// code by ntu ael

//To test ADCS and OBDH combination to do some issues:
// 1) Detumbling when it first ejected.
// 2) Sun pointing all the time and when the satellite is in eclipse.
// 3) Nadir pointing with high accuracy as it wants to deploy the P-SAT.
// 4) Pointing to the wanted direction when it wants to capture a picture with the camera.
//
//To program the ADCS detumbling when it is in the LEOP. What kind of procedure that you are going to apply?
// 1) ADCS will be on -> enter Sun-Pointing Mode
// 2) Commanded by OBDH to enter detumbling mode for maximum of 2 orbits.
// 3) When the timeout or stable conditions reached, the ADCS will stop detumbling
//
//Sun pointing all the time, but ADCS must inform OBDH when it is in the Eclipse, for only ADCS knows it
// 1) to receive Eclipse information from ADCS (this should be included in the housekeeping).
//
//To retrieve information from ADCS of its position error (this should be included in the housekeeping)
//
//To point the satellite into the wanted direction and then combine it with Payload activation.
// 1) If the power is considered "sufficient" to perform the action
// 2) Then turn ON ADCS and let ADCS to perform certain orientation. I do not know how long does this will take. But I think it will be till 2 minutes after the appointed time
// 3) Then turn ON the camera on the appointed time (or before) and let it ON for a while
// 4) Then the camera will capture the image for two minutes, ADCS will be back to the Sun-pointing mode
// 5) Collect the camera data in a rotating slot no.

#include "obdh_main.h"

char sfrStack[8];

char UART0_rx_buffer[CQT_UART_RX_SIZE]; // cqt
char UART0_tx_buffer[32];

#if 255 <= CQT_UART_RX_SIZE
#error "cqt uart rx size must be strictly less than 255"
#endif

char UART1_rx_buffer[128]; // uart1 is for debug
char UART1_tx_buffer[480];

extern State mainState;

void main() {
  // at gear0/1 no uart1 is possible since the smbus is very busy (interrupt priorities)
  OscGear osc_gear = oscillator_gear4; // at gear4 the operation of cqt only produces around 2 write, and around 2 read sd card errors
  Watchdog_disable();
  Oscillator_init(osc_gear);
  SFR_init(sfrStack, sizeof sfrStack); // Port_init() uses sfr_stack
  Port_init();
  Meta_init(); // required for chip_meta.systemFreq

  // --- c8051
  SPI0_init(chip_meta.systemFreq, F_SCK_MAX, SPI0_CONFIG, SPI0_TIMEOUT);
  Timer0_init(chip_meta.systemFreq, System_getCKCON(osc_gear)); // not used, but initialized to prevent floating state
  UART1Timer1_init(chip_meta.systemFreq, System_getT1Prescaler(osc_gear), BAUDRATE_1); // used for UART1 baudrate generator
  Timer2_init(chip_meta.systemFreq); // used for watchdog and various delays
  UART0Timer4_init(chip_meta.systemFreq, BAUDRATE_0); // used for UART0 baudrate generator
  UART0_open(UART0_rx_buffer, sizeof UART0_rx_buffer, UART0_tx_buffer, sizeof UART0_tx_buffer, 0x0f);
  UART1_open(UART1_rx_buffer, sizeof UART1_rx_buffer, UART1_tx_buffer, sizeof UART1_tx_buffer);
  ADC0_init(chip_meta.systemFreq, VREF_MV);
  SMBus_setRxBuffer(i2c_rxbuffer, sizeof i2c_rxbuffer);
  SMBus_init(OBDH_ADDRESS, chip_meta.systemFreq);
  SMBusTimer3_init(chip_meta.systemFreq); // Initialize Timer 3: used for I2C SCL timeout (SMBus)

  // --- velox
  Checksum_init(CHECKSUM_PLY);
  Watchdog_advancedInit(IE_TIMER2_MASK, WATCHDOG_TIMEOUT_MS); // 10s watchdog timeout, one main loop cycle should take less than 10s (typically < 30 ms)
  EA = 1; // after watchdog init, but before have to enable interrupts before the first putchar!

  Auth_init();
  Debug_init();
  Timekeeper_init();
  Comm_init(i2c_txbuffer, i2c_rxbuffer);

  Rtc_init(i2c_txbuffer, i2c_rxbuffer);
  Uptime_init(TAI_TIME_UNAVAILABLE, 0, 300); // 300 sec as storing interval
  Schedule_init(); // initialize the schedule
  Cqt_init();

  StateUtil_init(i2c_txbuffer, i2c_rxbuffer, sizeof i2c_rxbuffer);
  State_initStatetask(&g_statetask_pwrsHk, 503); // interaction with PWRS
  State_initStatetask(&g_statetask_adcsHk, 3907); // interaction with ADCS (intentionally much less frequent than with PWRS)
  State_initStatetask(&g_statetask_trxuvHk, 473);
  State_initStatetask(&g_statetask_xdatSegGet, 83);
  State_initStatetask(&g_statetask_wireUp, 47); // fast for mccif uplink via Uart, state allows user to control obdh, state is passive unless specifically commanded
  State_initStatetask(&g_statetask_radioUp, 827);
  State_initStatetask(&g_statetask_cmdHandler, 137);
  State_initStatetask(&g_statetask_scheduler, 521); // execute what is in the schedule
  State_initStatetask(&g_statetask_downlink, STATE_DONWLINK_DELAY_MIN); // downlink state is passive unless specifically commanded
  State_initStatetask(&g_statetask_ferryCam, 67); // only active when sbc and csfb channels are powered up, see Recon_ChannelStatusUpdated_Event()
  State_initStatetask(&g_statetask_ferryXBee, 373);
  State_initStatetask(&g_statetask_ferryGps, 443);
  State_initStatetask(&g_statetask_rtcSync, 60001); // 1009ms is for testing. 1 minute is more than sufficient for RTC synchronization (handled in State_rtcSync)
  State_initStatetask(&g_statetask_powerCqt, 631);

  g_statetask_powerCqt.task.mode = timertask_disabled; // auto activated by cqt_handler()

  Constant_init();
  Trxuv_init();

  SD_initConfig();
  SDred_init(Mccif_isValidContent, readbackcopy);

  State_starter(&mainState, 800); // any statetask is not allowed to run for more than 0.8 sec

  //Hash_readManifest(); //At least must return to the original state! how??

  Watchdog_lockout();
  while (1) {

    if (Clock_hasAdvanced()) { // true when ms count is different at this point

      if (Seconds_haveAdvanced()) { // true once per second
        Grace_eachSecond();
        Timekeeper_handler();
      }

      Meta_handler(); // gives states the latest chip info

      State_turing(&mainState, &g_statetask_pwrsHk, State_pwrsHk);
      State_turing(&mainState, &g_statetask_adcsHk, State_adcsHk);
      State_turing(&mainState, &g_statetask_trxuvHk, State_trxuvHk);
      State_turing(&mainState, &g_statetask_xdatSegGet, State_xdatSegGet);
      State_turing(&mainState, &g_statetask_wireUp, State_wireUp);
      State_turing(&mainState, &g_statetask_radioUp, State_radioUp);
      State_turing(&mainState, &g_statetask_cmdHandler, State_cmdHandler);
      State_turing(&mainState, &g_statetask_scheduler, State_scheduler);
      State_turing(&mainState, &g_statetask_downlink, State_downlink);
      State_turing(&mainState, &g_statetask_ferryCam, State_ferryCct);
      State_turing(&mainState, &g_statetask_ferryXBee, State_ferryXBee);
      State_turing(&mainState, &g_statetask_ferryGps, State_ferryGps);
      State_turing(&mainState, &g_statetask_rtcSync, State_rtcSync);
      State_turing(&mainState, &g_statetask_powerCqt, State_powerCqt);

      Debug_loopDownlink();

      Cqt_handler();
    } // end of ms entry
  }
}
