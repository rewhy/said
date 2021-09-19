// config.h
#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdio.h>
#include <pthread.h>

#include "utils.h"
#include "isoTp.h"
#include "portControl.h"

#if 0
#define TARGET_ID_NUM 4

/* The index of the in-vehicle systems */
typedef enum {
	CAN_ID_ECE_ACTION_IDX = 0,  // Electronic central electrical system
	CAN_ID_LFD_ACTION_IDX = 1,  // Left front door system
	CAN_ID_RFD_ACTION_IDX = 2,  // Right front door system
	CAN_ID_DAS_ACTION_IDX = 3,  // Dash board system
	CAN_ID_ENG_ACTION_IDX = 4,  // Engine system
	CAN_ID_TCM_ACTION_IDX = 5,  // transmission (TCM) system
	CAN_ID_BRK_ACTION_IDX = 6,  // brake system
	CAN_ID_STE_ACTION_IDX = 7   // Steering system
} enumCanIdIndex;

#define TARGET_CAN_ID_NUM	8

/* The CAN ids of special CAN requests for the corresponding in-vehicle subsystems */
struct ivRequestCanIds {
	unsigned short req_can_ids[TARGET_CAN_ID_NUM];
	unsigned int	 can_id_num;
};

/* The CAN ids of special CAN responses for the corresponding in-vehicle subsystems */
struct ivResponseCanIds {
	unsigned short rep_can_ids[TARGET_CAN_ID_NUM];
	unsigned int	 can_id_num;
};


/* The special/target requests for retrieving in-vehicle context */
struct ivContextRequests {
	struct ivCanFrameInfo obd_ack;
	struct ivCanFrameInfo get_vin;
	struct ivCanFrameInfo get_data;
	struct ivCanFrameInfo new_session;
	struct ivCanFrameInfo frame_control;
	struct ivCanFrameInfo quit_session;
};

/* Contain the information of the special CAN responses */
struct ivContextResponses {
	struct ivCanFrameInfo new_session; // The response of successful session construction
	struct ivCanFrameInfo quit_session; // TODO: The response of successful leaving session
};

/* The special/target requests for performing special actions */
struct ivActionRequests {
	struct ivCanFrameInfo blare_horn1;
	struct ivCanFrameInfo blare_horn2;
	struct ivCanFrameInfo unlock;
	struct ivCanFrameInfo lock;
	struct ivCanFrameInfo open_window;
	struct ivCanFrameInfo close_window;
	struct ivCanFrameInfo open_rvm;
	struct ivCanFrameInfo close_rvm;
};

void initVehicleRequests(struct ivContextRequests *cxt_req, struct ivActionRequests *act_req);
void initVehicleResponses(struct ivContextResponses *cxt_rep);
void initVehicleCanIds(struct ivRequestCanIds *req_ids, struct ivResponseCanIds *ack_ids);
void initVehicleSpecificInfo(void); // TODO: the aboving initialization functions will be invoked in individual module


struct ivActionCtrl* addVehicleActionInfo(struct ivActionCtrl *head, uint req_id, 
		uchar *req_data, uint req_len, uint ack_id, uchar *ack_data, uint ack_len);
#endif

struct ivSysInfo* initIVContextInfo(char *csv_context) ;
struct ivActionCtrl* initIVActionInfo(char *csv_action);
struct ctrlAction* initControlRules(char *csv_actonRule);


#endif // __CONFIG_H
