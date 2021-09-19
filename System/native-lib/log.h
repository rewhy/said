#ifndef __LOG_H
#define __LOG_H

#include <linux/can.h>

#define MAX_TMP_BUF	1024

int initLog(void);

int logSensor(double *data);
int logRxFrame(struct can_frame *frame);
int logTxFrame(struct can_frame *frame);
int myLog(const char *fmt, ...);
void exitLog(void);
#endif// _LOG_H
