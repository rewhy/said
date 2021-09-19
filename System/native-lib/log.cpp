// log.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <linux/string.h>
#include <utils/Log.h>

#include "utils.h"
#include "log.h"

#define DATATEST 1

static FILE *log_fd = NULL;
static pthread_mutex_t	log_lock;

int initLog()
{
    int i, j;
    char file_path[255];


    if(log_fd == NULL)
    {
        sprintf(file_path, "/sdcard/canitf/%llu.log", getCurrentTime());

        ALOGI("Initial log file %s", file_path);		

        log_fd = fopen(file_path, "w");
        if(log_fd == NULL)
        {
            ALOGE("Error occured while opening file %s, exiting...\n", file_path);
            return 0;
        }
    }
    pthread_mutex_init(&log_lock, NULL);
    return 1;
}

int logSensor(double *data)
{
    int n = 0;
    char buf[MAX_TMP_BUF];

    if(log_fd == NULL)
    return 0;


    memset(buf,  0, MAX_TMP_BUF);

    n = sprintf(buf, "[OB@%llu]: %f, %f, %f, %f, %f, %f \t\n", getCurrentTime(), 
                data[0], data[1], data[2], data[3], data[4], data[5]);

    pthread_mutex_lock(&log_lock);
    fwrite(buf, n, 1, log_fd);
    pthread_mutex_unlock(&log_lock);

    return n;  
} 

int logRxFrame(struct can_frame *frame)
{
    int n = 0;
    char buf[MAX_TMP_BUF], buf1[MAX_TMP_BUF];

    if(log_fd == NULL)
    return 0;


    memset(buf,  0, MAX_TMP_BUF);
    memset(buf1, 0, MAX_TMP_BUF);

    bytesToHexString(buf, frame->data ,frame->can_dlc);
    n = sprintf(buf1, "[RX@%llu]: 0x%03x, %s\t\n", getCurrentTime(), frame->can_id, buf);

    pthread_mutex_lock(&log_lock);
    fwrite(buf1, n, 1, log_fd);
    pthread_mutex_unlock(&log_lock);

    return n;  
} 

int logTxFrame(struct can_frame *frame)
{
    int n = 0;
    char buf[MAX_TMP_BUF], buf1[MAX_TMP_BUF];

    if(log_fd == NULL)
    return 0;


    memset(buf,  0, MAX_TMP_BUF);
    memset(buf1, 0, MAX_TMP_BUF);

    bytesToHexString(buf, frame->data ,frame->can_dlc);
    n = sprintf(buf1, "[TX@%llu]: 0x%03x, %s\t\n", getCurrentTime(), frame->can_id, buf);

    pthread_mutex_lock(&log_lock);
    fwrite(buf1, n, 1, log_fd);
    pthread_mutex_unlock(&log_lock);

    return n;  
} 

int myLog(const char *fmt, ...)
{
    va_list ap;
    char buf[MAX_TMP_BUF], buf1[MAX_TMP_BUF];
    int n = 0;

    if(log_fd == NULL)
    return 0;

    memset(buf, 0, MAX_TMP_BUF);
    memset(buf1, 0, MAX_TMP_BUF);
    pthread_mutex_lock(&log_lock);

    va_start(ap, fmt);
    n = vsprintf(buf, fmt, ap);
    va_end(ap);

    n = sprintf(buf1, "[%llu] %s\t\n", getCurrentTime(), buf);
    ALOGI("Log: %s", buf);
    fwrite(buf1, n, 1, log_fd);

    pthread_mutex_unlock(&log_lock);

    return n;  
} 

void exitLog(void)
{
    myLog("Exit@%llu", getCurrentTime());
    fclose(log_fd);
    pthread_mutex_destroy(&log_lock);
}
