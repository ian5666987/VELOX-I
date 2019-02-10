// code by ntu ael

#ifndef COMM_IMC_H
#define COMM_IMC_H

struct struct_imc { // sizeof = 8 * 2 + 3 = 19
  short doppler_offset;
  short rx_strength;
  short tx_reflection;
  short tx_forward;
  short tx_current;
  short rx_current;
  short amp_temperature;
  short bus_voltage;
  char rx_uptime_minutes;
  char rx_uptime_hours;
  char rx_uptime_days;
};

extern struct struct_imc g_imc;

void Comm_rxReset();

void Comm_rxQueueSizePut();
char Comm_rxQueueSizeGet(char* numel);

void Comm_rxFramePut();
char Comm_rxFrameGet(void* buffer, char* length);
void Comm_rxFrameDrop();

void Comm_allTelemetryPut();
char Comm_allTelemetryGet();

void Comm_rxUptimePut();
char Comm_rxUptimeGet();

#endif
