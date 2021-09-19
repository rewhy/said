// portControl.c

/*
 * This file is used to implement functions 
 * that send special UDS request to perform special actions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <time.h>
#include <string.h>

#include "utils.h"
#include "log.h"
#include "canitf.h"
#include "obdPort.h"
#include "vehicle.h"
#include "clientCom.h"
#include "portContext.h"
#include "portControl.h"

#include "ivSpec.h"



extern struct sysRunningStatus *sys_state;
extern struct IVSysInfo	*iv_sys_list;

static struct ivActionCtrl *iv_action_list = NULL;

struct ivActionCtrl *ext_iv_act;
struct ctrlAction *action_rules;
/*
 * #data# contains the argument
 */

static void doActionByName(const char *name, uchar *data) {
	/* Retrieve the context relavent to special in-vehicle system */

	int old_mode = sys_state->run_mode;
	struct ivActionCtrl *iv_act = iv_action_list;
	struct IVSysInfo	*iv_sys = used_sys_list;


	ALOGD("To do action by name: %s", name);
	while(iv_act) {
		ALOGD("Compar name: %s %s", name, iv_act->name);
		if(strcmp(iv_act->name, name) == 0)
			break;
		iv_act = iv_act->next;
	}

	if(iv_act == NULL) {
		ALOGI("Cannot find the target action %s", name);
		return;
	}
#if IS_NOT_SIMULATOR

	while(iv_sys) {
		if(iv_sys->req_id == iv_act->req_id) {
			break;
		}
		iv_sys = iv_sys->next;
	}

	if(iv_sys == NULL) {
		ALOGI("Cannot find the target sys info for action %s", name);
		return;
	}

	// To build a new session with the specified in-vehicle system
	if(!buildNewSession(iv_sys->req_id, iv_sys->ack_id, iv_sys)) {
		//ALOGD("Build session failure!!!");
		return false;
	} else {
		sys_state->run_mode = SYS_MODE_ACTION;
	}
#else
	sys_state->run_mode = SYS_MODE_ACTION;
#endif
    usleep(50*1000);
	//normal action
#if IS_NOT_SIMULATOR
	ALOGD("LYY123 start to txPayload: len %d data 0x%08x",iv_act->req_len,iv_act->req_data);
	uint tx_len = txPayload(sys_state->can0_info->can_sk, sys_state->can0_info->can_src_id, 
			sys_state->can0_info->can_dst_id, iv_act->req_data, iv_act->req_len); // Tx the request
	ALOGD("LYY123 end to tx");
#else
	//special action
	uint tx_len = iv_act->req_len;
	txCanFrame(sys_state->can0_info->can_sk,iv_act->req_id,iv_act->req_data,iv_act->req_len);
#endif

	if(tx_len != iv_act->req_len) {
		ALOGE("Transmitting action request payload error!!!");
	} else {
		usleep(100 * 1000);
	}
	// Close the session :no need for special suitation 
#if IS_NOT_SIMULATOR
	quitCurrentSession(iv_sys);
#endif
	sys_state->run_mode = old_mode;
}

//if use function "random", must use setSeed() once at first
void setSeed(){
    srand((unsigned)(time(NULL)));
}

uint getRandomIVID(){
    uint id;
    id = rand()%1792;
    return id;
}

uint getRandomDiagnosisiID(){
    uint id;
    id = rand()%100+1792;
    return id;
}

int getRandomPayload(unsigned char* data,int max_size){
    memset(data,0,max_size);
    int len;
    len = rand()%8+1;
    for(int i=0;i<len;i++){
        data[i]=rand()%256;
    }
    return len;
}


void doInjectAction(){

}

void doControlAction(uchar action, uchar *data) {


	ALOGD("To do action: %d", action);

	switch(action) {
		case CMD_ACTION_HORN:
			doActionByName((char *)"horn", data);
			break;
		case CMD_ACTION_UNLOCK:
			doActionByName((char *)"unlock", data);
			break;
		case CMD_ACTION_LOCK:
			doActionByName((char *)"lock", data);
			break;
		case CMD_ACTION_OPEN_WINDOW:
			doActionByName((char *)"openwindow", data);
			break;
		case CMD_ACTION_CLOSE_WINDOW:
			doActionByName((char *)"closewindow", data);
			break;
		case CMD_ACTION_CLOSE_RVM:
			doActionByName((char *)"closervm", data);
			break;
		case CMD_ACTION_OPEN_RVM:
			doActionByName((char *)"openrvm", data);
			break;
		case CMD_ACTION_DIPPED_LIGHT:
			doActionByName((char *)"dipped_headlight", data);
			break;
		case CMD_ACTION_HIGH_BEAM:
			doActionByName((char *)"high_beam", data);       
			break;
		case CMD_ACTION_TURN_SIGNAL_LAMP:
			doActionByName((char *)"turn_signal_lamp", data);
			break;
		case CMD_ACTION_FOGLIGHT:
			doActionByName((char *)"foglight", data);
			break;
		case CMD_ACTION_REARFOGLAMP:
			doActionByName((char *)"rearfoglamp", data);
			break;
		case CMD_ACTION_BRAKELAMP:
			doActionByName((char *)"brakelamp", data);
			break;
		case CMD_ACTION_CENTRALLOCKLIGHT:
			doActionByName((char *)"centrallockindicatorlight", data);
			break;
		case CMD_ACTION_REVERSINGLAMP:
			doActionByName((char *)"reversing_lamp", data);
			break;
		case CMD_ACTION_DOME_COLCK:
			doActionByName((char *)"dome_colck", data);
			break;
		case CMD_ACTION_WINDSCREEN_WASHERS:
			doActionByName((char *)"windscreen_washers", data);
			break;
		case CMD_ACTION_SIGNAL_HORN:
			doActionByName((char *)"signal_horn", data);
			break;
		case CMD_ACTION_FRONTDRIVERS_CLOSEWINDOW:
			doActionByName((char *)"frontdriverside_closewindow", data);
			break;
		case CMD_ACTION_FRONTDRIVERS_OPENWINDOW:
			doActionByName((char *)"frontdriverside_openwindow", data);
			break;
		case CMD_ACTION_REARDRIVERS_CLOSEWINDOW:
			doActionByName((char *)"reardriverside_closewindow", data);
			break;
		case CMD_ACTION_REARDRIVERS_OPENWINDOW:
			doActionByName((char *)"reardriverside_openwindow", data);
			break;
		case CMD_ACTION_FRONTPASSENGER_CLOSEWINDOW:
			doActionByName((char *)"frontpassenger_closewindow", data);
			break;
		case CMD_ACTION_FRONTPASSENGER_OPENWINDOW:
			doActionByName((char *)"frontpassenger_openwindow", data);
			break;
		case CMD_ACTION_REARPASSENGER_CLOSEWINDOW:
			doActionByName((char *)"rearpassenger_closewindow", data);
			break;
		case CMD_ACTION_REARPASSENGER_OPENWINDOW:
			doActionByName((char *)"rearpassenger_openwindow", data);
			break;
		case CMD_ACTION_LIGHTING:
			doActionByName((char *)"lighting", data);
			break;
		case CMD_ACTION_INDICATORLIGHT:
			doActionByName((char *)"indicatorlight", data);
			break;
		default:
			break;
	}
}


void initControlInfo() {
	iv_action_list = initIVActionInfo("/sdcard/skeda_action.csv");
	ext_iv_act = initIVActionInfo("/sdcard/skeda_action.csv");
	action_rules = initControlRules("/sdcard/action_rules.csv");
	if(action_rules==NULL){
		ALOGD("LYYA actionrule is null");
	}

	ALOGD("Initialize control info.");
#ifdef IS_ACTION_MODE
	sys_state->run_mode	= SYS_MODE_ACTION;
#endif
	//ALOGD("Set run_mode = 0x%x", SYS_MODE_ACTION);
}
