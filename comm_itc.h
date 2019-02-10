// code by ntu ael

#ifndef COMM_ITC_H
#define COMM_ITC_H

#define  COMM_TX_RATE_MASK        0x0C // 1100
#define  COMM_IDLE_STATUS_MASK    0x40

struct struct_itc {
  char tx_available;
  char tx_uptime_minutes;
  char tx_uptime_hours;
  char tx_uptime_days;
  char tx_status;
};

extern struct struct_itc g_itc;

void Comm_txReset();
void Comm_txAx25Put(void* buffer, short length);
char Comm_txAx25AndCwGet(char* numel);
void Comm_setTxIdleState(char value);
void Comm_setTxOutputMode(char value);
void Comm_setTxBitrate(char value);
void Comm_setTxCwCharRate(short value);
void Comm_txUptimePut();
char Comm_txUptimeGet();
void Comm_txStatusPut();
char Comm_txStatusGet();

#endif
