#ifndef _UTILS_H_
#define _UTILS_H_

//#include <unistd.h>

#define DBG_LOG


#ifdef	LOG_TAG
#undef  LOG_TAG
#endif

#define LOG_TAG "CANIPS"


#define ACTION_ATTACK_MODE

#define DBG_OUTPUT_FILE  "/sdcard/output.txt"

typedef unsigned char byte;
typedef unsigned char BYTE;
typedef unsigned int  UINT32;

typedef long long							llong;

typedef unsigned long long		ullong;
typedef unsigned long					ulong;
typedef unsigned int					uint;
typedef unsigned short				ushort;
typedef unsigned char					uchar;



void waitInCxt(int max_ns);

int get_pids_by_name(pid_t *pids, int max, const char *name);

void intToArray(int data, byte* buff);
int arrayToInt(byte* buff);
int hexStringToBytes(uchar *bdst, uchar *hstr, int len);
int String2Bytes(uchar *bdst, char *hstr, int len);
void bytesToHexString(char *hdst, uchar *bsrc, int len);
void bytes2String(char *hdst, uchar *bsrc, int len);
int cmp2uchar(uchar* a,uchar* b,int len1,int len2);

ullong getCurrentTime(void);

#endif//#ifndef _UTILS_H_
