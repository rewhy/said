// vehicle.c
//
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/Log.h>

#include "utils.h"
#include "log.h"
#include "canitf.h"
#include "isoTp.h"
#include "portContext.h"
#include "portControl.h"
#include "onboard.h"
#include "obdPort.h"
#include "vehicle.h"


#define XUELEI 0

#if 0
unsigned int TARGET_CAN_ID1[TARGET_ID_NUM] = {0x0000007E0, 0x000007E1, 0x00000713, 0x00000712};//,0x0000070E deleted
static uchar FDJDID1[8][2] = {{0x10,0x5C}, {0x12,0xB0}, {0xF4,0x0C}, {0xF4,0x0D}, {0x15,0xD1}, {0x16,0x33}, {0x15,0xCF}, {0x15,0xD0}};
//BYTE ZDDID[3][2] = {{0x38,0x03},{0x21,0x0E},{0x38,0x04}};
static uchar ZDDID1[2][2] = {{0x38,0x03}, {0x38,0x04}};
//BYTE SCDID[6][2] = {{0x18,0x08},{0x18,0x16},{0x18,0x00},{0x18,0x01},{0x18,0x02},{0x18,0x03}};
static uchar SCDID1[4][2] = {{0x18,0x00}, {0x18,0x01}, {0x18,0x02}, {0x18,0x03}};
//BYTE DQDID[4][2] = {{0x19,0x44},{0x19,0x48},{0x19,0x4C},{0x19,0x50}};
static uchar ZXDID1[1][2] = {{0x18,0x12}};
	
static uchar i_s_req[3] = { 0x05, 0x03, 0x00 };
static uchar q_s_req[1] = { 0xa8 };


extern struct ivSysInfo *iv_sys_list;
#endif
#if 0
static void initIvCanRequest(struct ivCanFrameInfo *req, unsigned int id, unsigned char *data, unsigned int len) {
	req->id			= id;
	req->length = len;
	memcpy(req->data, data, req->length);
}

/* Initialize the requests for special actions */
void initVehicleRequests(struct ivContextRequests *cxt_req, struct ivActionRequests *act_req) {
	unsigned char data[8];
	if (cxt_req == NULL or act_req == NULL) {
		ALOGD("Init requests error!!!!");
		return;
	}
	
	data[0] = 0x30;
	data[1] = 0x00;
	data[2] = 0x0a;
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&cxt_req->obd_ack, 0x7e0, data, 8);

	/* Flow Control (FC) Frame:
	 * An ISO-TP frame is always 8 bytes and unneeded bytes filed with the padding bytes as 0xAA or 0x55
	 */
	data[0] = 0x30; // FC frame (0x3), FS=0x0 which is flow status (0: ClearToSend, 1: Wait, 2:Overflow))
	data[1] = 0x00; // BS=0x00, which represents block size and shows how many consecutive frames need to be send in one block
	data[2] = 0x0a; // STmin=0x0a, which shows the minimum separation time between consecutive frames to be noticed
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&cxt_req->frame_control, 0x0, data, 8);
	
	data[0] = 0x02;
	data[1] = 0x09;
	data[2] = 0x02;
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&cxt_req->get_vin, 0x7df, data, 8);

	data[0] = 0x02;
	data[1] = 0x10;
	data[2] = 0x03;
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&cxt_req->new_session, 0xfff, data, 8); // id==fff means it is variable

	data[0] = 0xa8;
	initIvCanRequest(&cxt_req->quit_session, 0xfff, data, 1);

	data[0] = 0x03;
	data[1] = 0x22;
	data[2] = 0xff;
	data[3] = 0xff;
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&cxt_req->get_data, 0xfff, data, 8); // id==fff means it is variable

	/*============ Action request ============*/
	data[0] = 0x05;
	data[1] = 0x2f;
	data[2] = 0x09;
	data[3] = 0x66;
	data[4] = 0x03;
	data[5] = 0x01;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&act_req->blare_horn1, 0x70e, data, 8); 
	data[0] = 0x04;
	data[1] = 0x2f;
	data[2] = 0x09;
	data[3] = 0x66;
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&act_req->blare_horn2, 0x70e, data, 8); 
	
	data[0] = 0x10;
	data[1] = 0x00;
	data[2] = 0x2f;
	data[3] = 0x00;
	data[4] = 0x03;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x03;
	initIvCanRequest(&act_req->unlock, 0x74a, data, 8); 
	
	data[0] = 0x10;
	data[1] = 0x00;
	data[2] = 0x2f;
	data[3] = 0x00;
	data[4] = 0x03;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x01;
	initIvCanRequest(&act_req->lock, 0x74a, data, 8);

	/* To idenfity the arguments of controling each window */
	data[0] = 0x10;
	data[1] = 0x00;
	data[2] = 0x2f;
	data[3] = 0x00;
	data[4] = 0x02;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&act_req->open_window, 0x74a, data, 8);
	
	data[0] = 0x10;
	data[1] = 0x00;
	data[2] = 0x2f;
	data[3] = 0x00;
	data[4] = 0x01;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
	initIvCanRequest(&act_req->close_window, 0x74a, data, 8);
	
	data[0] = 0x10;
	data[1] = 0x00;
	data[2] = 0x2f;
	data[3] = 0x00;
	data[4] = 0x0c;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x01;
	initIvCanRequest(&act_req->close_rvm, 0x74a, data, 8);
	
	data[0] = 0x10;
	data[1] = 0x00;
	data[2] = 0x2f;
	data[3] = 0x00;
	data[4] = 0x0c;
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x02;
	initIvCanRequest(&act_req->open_rvm, 0x74a, data, 8);
}


static void initIvCanResponse(struct ivCanFrameInfo *rep, unsigned int id, unsigned char *data, unsigned int len) {
	rep->id			= id;
	rep->length = len;
	memcpy(rep->data, data, rep->length);
}

void initVehicleResponses(struct ivContextResponses *cxt_rep) {
	unsigned char data[8];

	data[0] = 0x05;
	data[1] = 0x03;
	data[2] = 0x00;
	initIvCanResponse(&cxt_rep->new_session, 0xfff, data, 3);
}
#endif

#if 0
	CAN_ID_ECE_ACTION_IDX = 1,  // Electronic central electrical system
	CAN_ID_LFD_ACTION_IDX = 2,  // Left front door system
	CAN_ID_RFD_ACTION_IDX = 3,  // Right front door system
	CAN_ID_DAS_ACTION_IDX = 4,  // Dash board system
	CAN_ID_ENG_ACTION_IDX = 5,  // Engine system
	CAN_ID_TCM_ACTION_IDX = 6,  // transmission (TCM) system
	CAN_ID_BRK_ACTION_IDX = 7,  // brake system
	CAN_ID_STE_ACTION_IDX = 8  // Steering system
#endif
#if 0
static void initRequestsOfSys(struct ivSysInfo *iv_sys, uchar **data, uint len) {
	struct cxtRequest *new_request = NULL, *tmp = NULL;
	for(uint i = 0; i < len; i++) {
		new_request = (struct cxtRequest*)malloc(sizeof(struct cxtRequest));
		if(new_request == NULL) {
			ALOGE("Malloc cxtRequest Error !!!");
			return;
		}
		memset((void*)new_request, 0, sizeof(struct cxtRequest));
		new_request->len = 2;
		memcpy(new_request->payload, data[i], 2);
		if(iv_sys->req_list == NULL) {
			iv_sys->req_list = new_request;
		} else {
			tmp = iv_sys->req_list;
			while(tmp->next) {
				tmp = tmp->next;
			}
			tmp->next = new_request;
		}
	} // for 
}
#endif
#if 0	
void initIVSysInfo(void) {
	struct ivSysInfo *head = NULL, *tmp = NULL, *new_iv_sys = NULL;	
	struct cxtRequest *new_request = NULL, *cxt_tmp = NULL;
	ushort req_id = 0, ack_id = 0;
	char buff[255];
	int  c_num = 0, r_num = 0, cxt_num = 0;
	char *field = NULL;
	
	for(uint i = 0; i < TARGET_ID_NUM; i++ ) {
		
		req_id = TARGET_CAN_ID1[i];
		
		new_iv_sys = (struct ivSysInfo*) malloc(sizeof(struct ivSysInfo));
		if( !new_iv_sys ) {
			ALOGE("Malloc ivSysInfo Error !!!");
			return;
		}
		memset((void*)new_iv_sys, 0, sizeof(struct ivSysInfo));
		
		new_iv_sys->request_id  = req_id;
		memcpy(new_iv_sys->build_session_request.data, i_s_req, 3);
		new_iv_sys->build_session_request.length = 3;
		memcpy(new_iv_sys->build_session_request.data, q_s_req, 1);
		new_iv_sys->leave_session_request.length = 1;
		
		if(req_id == 0x7e0) {
			new_iv_sys->ack_id = 0x7e8;
			strcpy(new_iv_sys->name, "FDJDID");
			ALOGD("New IV-SYS: %d, %d, %d, %s, %d, %s", new_iv_sys->request_id, new_iv_sys->ack_id, )
			initRequestsOfSys(new_iv_sys, (uchar**)FDJDID1, 8);
		} else if(req_id == 0x7e1) {
			new_iv_sys->ack_id = 0x7e9;
			initRequestsOfSys(new_iv_sys, (uchar**)ZDDID1, 2);
		} else if(req_id == 0x713) {
			new_iv_sys->ack_id = 0x77d;
			initRequestsOfSys(new_iv_sys, (uchar**)SCDID1, 4);
		}	else if(req_id == 0x712) {
			new_iv_sys->ack_id = 0x77c;
			initRequestsOfSys(new_iv_sys, (uchar**)ZXDID1, 1);
		}
		
		if(iv_sys_list == NULL) {
			iv_sys_list = new_iv_sys;
		} else {
			tmp = iv_sys_list;
			while(tmp->next) {
				tmp = tmp->next;
			}
			tmp->next = new_iv_sys;
		}
	} // for
}
#endif

/* 
 * Initial the CAN IDs of the special in-vehicle subsystems
 */
#if 0
void initVehicleCanIds(struct ivRequestCanIds *req_ids, struct ivResponseCanIds *ack_ids) {
	req_ids->req_can_ids[CAN_ID_ECE_ACTION_IDX] = 0x70E;
	ack_ids->rep_can_ids[CAN_ID_ECE_ACTION_IDX] = 0x778;
	
  req_ids->req_can_ids[CAN_ID_LFD_ACTION_IDX] = 0x74A;
	ack_ids->rep_can_ids[CAN_ID_LFD_ACTION_IDX] = 0x7B4;
	
	req_ids->req_can_ids[CAN_ID_RFD_ACTION_IDX] = 0x74B;
	ack_ids->rep_can_ids[CAN_ID_RFD_ACTION_IDX] = 0x7B5;
	
	req_ids->req_can_ids[CAN_ID_DAS_ACTION_IDX] = 0x714;
	ack_ids->rep_can_ids[CAN_ID_DAS_ACTION_IDX] = 0x77E;
	
	req_ids->req_can_ids[CAN_ID_ECE_ACTION_IDX] = 0x7E0;
	ack_ids->rep_can_ids[CAN_ID_ECE_ACTION_IDX] = 0x7E8;

	req_ids->req_can_ids[CAN_ID_TCM_ACTION_IDX] = 0x7E1;
	ack_ids->rep_can_ids[CAN_ID_TCM_ACTION_IDX] = 0x7E9;
	
	req_ids->req_can_ids[CAN_ID_BRK_ACTION_IDX] = 0x713;
	ack_ids->rep_can_ids[CAN_ID_BRK_ACTION_IDX] = 0x77D;
	
	req_ids->req_can_ids[CAN_ID_STE_ACTION_IDX] = 0x712;
	ack_ids->rep_can_ids[CAN_ID_STE_ACTION_IDX] = 0x77C;

	req_ids->can_id_num = 8;
	ack_ids->can_id_num = 8;
}

struct ivRequestCanIds		skeda_req_can_ids;   // vehicle-specific request  can ids
struct ivResponseCanIds		skeda_rep_can_ids;   // vehicle-specific response can ids

// struct ivCanFrameInfo skeda_response;
struct ivContextRequests	skeda_cxt_requests;  // vehicle-specific requests
struct ivContextResponses	skeda_cxt_responses; // vehicle-specific responses
struct ivActionRequests		skeda_act_requests;  // vehicle-specific requests

/* Initialize the special requests and frames from the configuration file */
void initVehicleSpecificInfo(void) {
	initIVSysInfo();
	//initVehicleCanIds(&skeda_req_can_ids, &skeda_rep_can_ids);
	//initVehicleRequests(&skeda_cxt_requests, &skeda_act_requests);
	//initVehicleResponses(&skeda_cxt_responses);
}

struct ivActionCtrl* addVehicleActionInfo(struct ivActionCtrl *head, uint req_id, uchar *req_data, uint req_len, uint ack_id, uchar *ack_data, uint ack_len) {
	
	struct ivActionCtrl *tmp = NULL;
	struct ivActionCtrl *new_node = (struct ivActionCtrl *)malloc(sizeof(struct ivActionCtrl));

	if(new_node == NULL) {
		ALOGE("Vehicle: Allocate new action node error!!!");
		return NULL;
	}
	
	new_node->req_id  = req_id;
	new_node->req_len = req_len;
	memcpy(new_node->req_data, req_data, req_len);
	new_node->ack_id  = ack_id;
	new_node->ack_len = ack_len;
	memcpy(new_node->ack_data, ack_data, ack_len);

	if( head != NULL ) {
		tmp = head;
		while(tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new_node;
	}

	ALOGD("Add Action: 00, %s, %d, %d, %s, %d, %d, %s\n", new_node->name,
			new_node->req_id, new_node->req_len, new_node->req_data,
			new_node->ack_id, new_node->ack_len, new_node->ack_data);

	return new_node;
}

#endif


#if XUELEI
struct ivSysInfo* initIVContextInfo(char *csv_context) {
	struct ivSysInfo *head = NULL, *tmp = NULL, *new_iv_sys = NULL;	
	struct cxtRequest *new_request = NULL, *cxt_tmp = NULL;
	ushort req_id = 0, ack_id = 0;
	char buff[255];
	int  c_num = 0, r_num = 0, cxt_num = 0, res = 0;
	char *field = NULL;
	char tmp1[MAX_TMP_BUF], tmp2[MAX_TMP_BUF];

	if(csv_context == NULL)
		return NULL;
	FILE *fp = fopen(csv_context, "r");

	if(fp == NULL) {
		ALOGE("Open Context CSV file (%s) error!!!", csv_context);
		return NULL;
	}

	while(fgets(buff, 255, fp)) {
		r_num++;
		if(r_num == 1) {
			ALOGI("Vehicle: Openned context CSV file of %s", buff); // the name of vehicle
			continue;
		}
	
		ALOGD("Read: %s", buff);
		
		new_iv_sys = (struct ivSysInfo*) malloc(sizeof(struct ivSysInfo));
		if( !new_iv_sys ) {
			ALOGE("Vehicle: Malloc ivSysInfo Error !!!");
			continue;
		}
		memset((void*)new_iv_sys, 0, sizeof(struct ivSysInfo));

		/* Parse one iv-sys row */
		c_num = 0;
		field = strtok(buff, ",");
		while(field) {
			if(c_num == 0) { // sys name
				ALOGI("Vehicle: Read IV-SYS %s!", field);
				strcpy(new_iv_sys->name, field);
			} else if(c_num == 1) { // req_id
				new_iv_sys->request_id = strtol(field, NULL, 16);
			} else if(c_num == 2) { // ack_id
				new_iv_sys->ack_id = strtol(field, NULL, 16);
			} else if(c_num == 3) { // syn req_len
				new_iv_sys->build_session_request.length = atoi(field);
			} else if(c_num == 4) { // syn req_data
				res = hexStringToBytes(new_iv_sys->build_session_request.data, (uchar*)field, MAX_LOAD_SIZE);
			} else if(c_num == 5) { // fin req_len
				new_iv_sys->leave_session_request.length = atoi(field);
			} else if(c_num == 6) { // fin req_data
				res = hexStringToBytes(new_iv_sys->leave_session_request.data, (uchar*)field, MAX_LOAD_SIZE);
			} else if(c_num == 7) { // No. of context types
				cxt_num = atoi(field);
			} else {
			}
			c_num++;
			field = strtok(NULL, ",");
		}

		bytesToHexString(tmp1, new_iv_sys->build_session_request.data, new_iv_sys->build_session_request.length);
		bytesToHexString(tmp2, new_iv_sys->leave_session_request.data, new_iv_sys->leave_session_request.length);
		ALOGD("New in-sys: %s, 0x%04x, 0x%04x, 0x%x, %s, 0x%x, %s, %d", new_iv_sys->name,
				new_iv_sys->request_id, new_iv_sys->ack_id,
				new_iv_sys->build_session_request.length, tmp1,
				new_iv_sys->leave_session_request.length, tmp2,
				cxt_num);

		/* Parsing context_request */
		while(cxt_num > 0) {

			if(!fgets(buff, 255, fp)) {
				ALOGE("Vehicle: Read context info error!!!");
				break;
			}
			ALOGI("READ: %s", buff);
			ALOGD("Read: %s", buff);
			r_num++;

			/* Parse on context row */
			c_num = 0;
			
			new_request = (struct cxtRequest*)malloc(sizeof(struct cxtRequest));
			if(new_request == NULL) {
				ALOGE("Malloc cxtRequest Error !!!");
				return NULL;
			}
			memset((void*)new_request, 0, sizeof(struct cxtRequest));
			
			field = strtok(buff, ",");
			while(field) {
				if(c_num == 0) { // Context name
					strcpy(new_request->name, field);
				} else if(c_num == 1) { // req_len
					new_request->len = atoi(field);
				} else if(c_num == 2) { // req_data
					res = hexStringToBytes(new_request->payload, (uchar*)field, MAX_LOAD_SIZE);
				} else if(c_num == 3) { // ack_len (useless)
				} else if(c_num == 4) { // ack_data (useless)
				} else { // error (useless)
				}
				c_num++;
				field = strtok(NULL, ",");
			}

			/* Add the context request to list of the iv-system */
			if(new_iv_sys->req_list == NULL) {
				new_iv_sys->req_list = new_request;
			} else {
				cxt_tmp = new_iv_sys->req_list;
				while(cxt_tmp->next) {
					cxt_tmp = cxt_tmp->next;
				}
				cxt_tmp->next = new_request;
			}
			bytesToHexString(tmp1, new_request->payload, new_request->len);
			ALOGD("New cxt-info: %s, %d, %s", new_request->name,
					new_request->len, tmp1);
			cxt_num--;
		} // while of add in-vehicle context request

		/* Add the in-system info to the list */
		if(head == NULL) {
			head = new_iv_sys;
		} else {
			tmp = head;
			while(tmp->next) {
				tmp = tmp->next;
			}
			tmp->next = new_iv_sys;
		}
	} // while parsing CSV file
	return head;
}
#endif

struct ctrlAction* initControlRules(char *csv_actonRule){
	
	struct ctrlAction *head = NULL, *tmp = NULL, *new_node = NULL;
	char buff[255];
	char *field;

	if(csv_actonRule == NULL){
		ALOGE("LYYERROR Not find the file");
		return NULL;
	}
	FILE *fp = fopen(csv_actonRule,"r");
	if(!fp){
		ALOGE("LYYERROR Cannot open the file");
	}

	while(fgets(buff,255,fp)){
		ALOGD("LYYREAD read:%s",buff);
		
		new_node = (struct ctrlAction *)malloc(sizeof(struct ctrlAction));
		if(new_node==NULL){
			ALOGE("LYYERROR action rules malloc error!");
			return NULL;
		}
		memset((void *)new_node,0,sizeof(struct ctrlAction));
		
		field = strtok(buff,",");
		int index=0;
		while(field){
			
			switch(index){
				case 0:
					strcpy(new_node->name,field);
					ALOGD("LYYREAD name:%s",new_node->name);
					break;
				case 1:
					new_node->decision = atoi(field);
					break;
				case 2:
					new_node->number = atoi(field);
					break;
			}
			if(index>=3){
				if(new_node->number<1){
					ALOGE("LYYERROR no rules!");
					break;
				}else{
					if(index%3==0){
						new_node->pid[index/3-1]=atoi(field);
						ALOGD("LYYREAD pid:%d",new_node->pid[index/3-1]);
					}else if(index%3==1){
						new_node->symbol[index/3-1] = atoi(field);
						ALOGD("LYYREAD symbol:%d",new_node->symbol[index/3-1]);
					}else if(index%3==2){
						new_node->value[index/3-1] = atof(field);
						ALOGD("LYYREAD value:%f",new_node->value[index/3-1]);
					}
				}	
			
			}

			index++;
			field = strtok(NULL,",");
		}
		if(index%3!=0){
			ALOGE("LYYERROR Format error! Index:%d",index);
			continue;
		}

		if(head == NULL) {
			head = new_node;
		} else {
			tmp = head;
			while(tmp->next) {
				tmp = tmp->next;
			}
			tmp->next = new_node;
		}
	}

	fclose(fp);
	return head;
}

struct ivActionCtrl* initIVActionInfo(char *csv_action){
	
	struct ivActionCtrl *head = NULL, *tmp = NULL, *new_node = NULL;
	char	buff[255];
	char	*field = NULL;
	uint  r_num = 0, c_num = 0;
	int   res;
	
	if(csv_action == NULL) {
		return NULL;
	}
	FILE *fp = fopen(csv_action, "r");
	if(!fp) {
		ALOGE("Vehicle: Cannot open file %s", csv_action);
		return NULL;
	}

	r_num = 0; // Rows
	while(fgets(buff, 255, fp)) {
		
		ALOGD("Vehicle READ: %s", buff);
		
        r_num++;
		if(r_num == 1) {
			ALOGI("Vehicle: Openned action CSV file of %s", buff); // the name of vehicle
			continue;
		}

		new_node = (struct ivActionCtrl *)malloc(sizeof(struct ivActionCtrl));
		if(new_node == NULL) {
			ALOGE("Vehicle: Allocate new action node error!!!");
			break;
		}

        memset((void*)new_node, 0, sizeof(struct ivActionCtrl));
			
		//ALOGD("Read: %s", buff);

		/* Parse one row */
		field = strtok(buff, ",");
		c_num = 0; // Column
		while(field) {
			if(c_num == 0) { // action name
				ALOGI("Vehicle: Read action %s!", field);
				strcpy(new_node->name, field);
			} else if(c_num == 1) { // index
				new_node->action_id = atoi(field);
			} else if(c_num == 2) { // req_id
				new_node->req_id = strtol(field, NULL, 16);
			} else if(c_num == 3) { // req_len
				new_node->req_len = atoi(field);
			} else if(c_num == 4) { // req_data
				res = hexStringToBytes(new_node->req_data, (uchar*)field, MAX_LOAD_SIZE);
			} else if(c_num == 5) { // ack_id
				new_node->ack_id = strtol(field, NULL, 16);
			} else if(c_num == 6) { // ack_len
				new_node->ack_len = atoi(field);
			} else if(c_num == 7) { // ack_data
				res = hexStringToBytes(new_node->ack_data, (uchar*)field, MAX_ACK_SIZE);
			} else { // Error
				ALOGE("Vehicle: Unknown action collumn (%s)!!!", field);
			}
			
			c_num++;
			field = strtok(NULL, ",");
		}
		
		if(c_num != 8) {
			ALOGE("Vehicle: Parse action CSV file error (line: %d)!!!", r_num);
			free(new_node);
			continue;
		}
		
		ALOGD("Vehicle New Action: %d, %d, %s, %d, %d, %s, %d, %d, %s\n", 
				c_num, new_node->action_id, new_node->name,
				new_node->req_id, new_node->req_len, new_node->req_data,
				new_node->ack_id, new_node->ack_len, new_node->ack_data);

		if(head == NULL) {
			head = new_node;
		} else {
			tmp = head;
			while(tmp->next) {
				tmp = tmp->next;
			}
			tmp->next = new_node;
		}
	}

	fclose(fp);
	return head;
}


