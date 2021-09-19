// ivSpec.h
#ifndef __IV_SPEC_H
#define __IV_SPEC_H

#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h> 

#include "utils.h"
#include "isoTp.h"

#define MAX_FILE_PATH		255
#define MAX_CAN_PAYLOAD	512
#define MAX_SIGNAL_NUM 10

struct DBCItem {
	char vec_name[255];

	char msgId[10];
	char msgName[128];
	char msgSize[10];

	char msgTransmitter[20];
	
	int signalNumber = 0;
	char* signalNameList[10];
	char* signalStartBitList[10];
	char* signalSizeList[10];
	char signalFormat[10];
	char signalSignedOrUnsigned[10];
	char* signalFactorList[10];
	char* signalOffsetList[10];
	char* signalUnitList[10];
	
	unsigned short req_id; // The functional reqeust id
	unsigned short ack_id; // The functional response id
	unsigned char* (*funGenRequest)(char* DID,struct IVReqInfo* req_info);  // The function for generating request
	bool (*funParResposne)(DBCItem* currentDBCItem, unsigned char* responseMsg,int len, struct IVCxtInfo* ctx_info); // The function for parsing the response
	struct DBCItem *next;
};

struct IVCxtInfo {
	//struct DBCItem *dbc_item;
	//struct timeval ts;
	//unsigned int	 req_len;		 // The length of the request data
	unsigned int	 ack_len;		 // The length of the request data
//	unsigned char	 req_data[MAX_CAN_PAYLOAD]; // The buf storing the request data
	unsigned char	 ack_data[MAX_CAN_PAYLOAD]; // The buf storing the response data
	
	float result;
	char sensorName[100];
	char sensorUnit[10];
};

struct IVReqInfo{
	char name[128];
	int req_len;
	unsigned char req_data[MAX_CAN_PAYLOAD];
	ushort req_id;
	ushort ack_id;
	struct IVReqInfo *next;
	bool flag;
	int frequence;
};

struct IVSysInfo{
	char name[128];
	ushort req_id;
	ushort ack_id;

	struct ivCanFrameInfo build_session_request;
	struct ivCanFrameInfo leave_session_request;
	struct IVReqInfo *req_list;
	struct IVSysInfo *next;
};

struct IVAckInfo{
	int ack_len;
	unsigned char ack_data[MAX_CAN_PAYLOAD];

	char name[128];
	char unit[100];
	float result;
};


//external interface
int initSpec(char* dbc_file);
struct DBCItem* getDBCItem(char* vec_name);
void genIVCxtReq(struct DBCItem* dbc_item);
bool parseIVCxtAck(struct IVCxtInfo* cxt_info,unsigned char* testResp,int len);
void finiSpec();

void initUDSInfo();

extern struct DBCItem *dbc_item_list;
extern struct IVCxtInfo *ack_info;
extern struct IVReqInfo *all_req_info;
//extern struct IVReqInfo *used_req_info;
extern struct IVSysInfo *used_sys_list;

#endif // __IV_SPEC_H



