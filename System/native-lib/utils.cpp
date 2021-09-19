#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <string>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <typeinfo>
#include <cutils/log.h>

#include "canitf.h"
#include "log.h"
#include "utils.h"
// #include "privateDataType.h"
//
extern struct sysRunningStatus *sys_state;

void waitInCxt(int max_us) {
	int n = max_us / 1000;
	for(int i = 0; i < n; i++) {
		usleep(1000);
		if((sys_state->run_mode & SYS_MODE_CONTEXT) == 0) {
			return;
		}
	}
}

int get_pids_by_name(pid_t *pids, int max, const char *name) {
	pid_t pid = -1, cpid = -1;
	DIR *dir = NULL;
	struct dirent *d = NULL;
	char filename[FILENAME_MAX], line[256];
	char pname[256], *p = NULL;
	FILE *fp = NULL;
	int num = 0;

	cpid = getpid();
	if (cpid < 0) {
		return -2;
	}

	dir = opendir("/proc");
	if (dir == NULL) {
		return -3;
	}
	while ((d = readdir(dir)) != NULL) {
		//        pid = (typeof(pid)) strtoul(d->d_name, NULL, 0);
		pid = (pid_t) strtoul(d->d_name, NULL, 0);
		if ((pid <= 0) || (pid == cpid)) { // avoid current
			continue;
		}
		snprintf(filename, sizeof(filename), "/proc/%s/status", d->d_name);
		fp = fopen(filename, "r");
		if (fp == NULL) {
			continue;
		}
		p = fgets(line, sizeof(line) - 1, fp);
		fclose(fp);
		if (p == NULL) {
			continue;
		}
		// line contain a string like "Name: binary_name"
		sscanf(line, "%*s %s", pname);
		if (name != NULL) {
			if (strcmp(pname, name) != 0) {
				continue;
			}
		}
		if (pids != NULL) {
			*pids++ = pid;
		}
		num++;
		if ((max >= 0) && (num >= max)) {
			break;
		}
	}
	closedir(dir);

	return num;
}

void intToArray(int data, byte* buff) {
	buff[0] = data & 0x000000ff;
	buff[1] = (data & 0x0000ff00) >> 8;
	buff[2] = (data & 0x00ff0000) >> 16;
	buff[3] = (data & 0xff000000) >> 24;
}

int arrayToInt(byte* buff) {
	return ((int)buff[0] | (int)buff[1] << 8 | (int)buff[2] << 16 | (int)buff[3] << 24);
}

static uchar charToByte(char c) {
	if(c >= '0' && c <= '9')
		return ((uchar)c - (uchar)'0');
	else if(c >= 'a' && c <= 'f')
		return ((uchar)c - (uchar)'a' + (uchar)10);
	else if(c >= 'A' && c <= 'F')
		return ((uchar)c - (uchar)'A' + (uchar)10);
	else
		return (uchar)0;
}

int hexStringToBytes(uchar *bdst, uchar *hstr, int len) {
	int h_size = strlen(hstr) / 2;
	int b_size = h_size > len ? len : h_size;
	uchar hhex, lhex;
	for(int i = 0; i < b_size; i++) {
		hhex = (uchar)charToByte(hstr[2*i]);
		lhex = (uchar)charToByte(hstr[2*i+1]);
		bdst[i] = (hhex << 4) | lhex;
	}
	return b_size;
}

int String2Bytes(uchar *bdst, char *hstr, int len){
    int h_size = (strlen(hstr) + 1) / 3;
	int b_size = h_size > len ? len : h_size;
	uchar hhex, lhex;
	for(int i = 0; i < b_size; i++) {
		hhex = (uchar)charToByte(hstr[3*i]);
		lhex = (uchar)charToByte(hstr[3*i+1]);
		bdst[i] = (hhex << 4) | lhex;
	}
	return b_size;

}


int cmp2uchar(uchar* a,uchar* b,int len1,int len2){
	
	//int len3 = strlen(*a);
	//ALOGD("LYY len3:",len3);
	ALOGD("LYY frame length:%d / %d",len1,len2);
	if(len1!=len2){
		return 0;
	}else{
		for(int p=0;p<len1;p++){
			if(a[p]!=b[p]){
				return 0;
			}
		}
		return 1;
	}
}



void bytesToHexString(char *hdst, uchar *bsrc, int len) {
	static char dict[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	uchar h_idx = 0, l_idx = 0;
	int i = 0;

	if(len == 0) {
		hdst[0] = '\0';
		return;
	}
	
	for(i = 0; i < len; i++){
		
		h_idx = (bsrc[i] & 0xf0) >> 4;
		l_idx = bsrc[i] & 0xf;

		hdst[3*i]   = dict[h_idx];
		hdst[3*i+1] = dict[l_idx];
		hdst[3*i+2] = '/';
	}
	if(i > 0)
		hdst[3*i-1] = '\0';
	
	//ALOGD("%s: %d %s %s", __func__, i, dict, hdst);
}

void bytes2String(char *hdst, uchar *bsrc, int len) {
	static char dict[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	uchar h_idx = 0, l_idx = 0;
	int i = 0;

	if(len == 0) {
		hdst[0] = '\0';
		return;
	}
	
	for(i = 0; i < len; i++){
		
		h_idx = (bsrc[i] & 0xf0) >> 4;
		l_idx = bsrc[i] & 0xf;

		hdst[2*i]   = dict[h_idx];
		hdst[2*i+1] = dict[l_idx];
	}
	if(i > 0)
		hdst[2*i] = '\0';

	//ALOGD("%s: %d %s %s", __func__, i, dict, hdst);
}

ullong getCurrentTime(void) {
	struct timespec tms;
	clock_gettime(CLOCK_REALTIME, &tms);
	return (ullong)((ullong)tms.tv_sec * 1000000 + (ullong)tms.tv_nsec/1000);
}

/*============ LOG ============*/
#if 0
void printBuffer(const char* name, unsigned char *buff, uint length) {
	ALOGD("%s:", name);
	char str[200];
	byte offset = 0;
	byte lineNum = 0;
	byte len = 0;
	memset(str, 0, sizeof(str));
	for (uint i = 0; i < length; i++) {
		len = sprintf(str + offset, " %02x", buff[i]);
		offset += len;
		if (i % 8 == 7) {
			len = sprintf(str + offset, "  ");
			offset += len;
		}

		if (i % 16 == 15) {
			ALOGD("L%d->%s", lineNum++, str);
			offset = 0;
			memset(str, 0, sizeof(str));
		}
	}
	ALOGD("L%d->%s", lineNum++, str);
}
#endif
