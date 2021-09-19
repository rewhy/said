// canitf.h
#ifndef __CAN_ITF_H
#define __CAN_ITF_H

#include <pthread.h>

#include "utils.h"
#include "onboard.h"
#include "vehicle.h"
#include "clientCom.h"
#include "obdPort.h"

// undef it if running in IPS mode
// #define IS_ACTION_MODE

#define IS_NOT_SIMULATOR 1
#define TEST 0

/* THe running mode of this system and each bit represents a individual mode */
typedef enum {
	SYS_MODE_IDLE		  = 0,        // No Client working
	SYS_MODE_ACTION		= 1,				// The system running as a remote control (attack) tool
	SYS_MODE_FORWARD	= 2,				// The system running as a branch
	SYS_MODE_SNIFFER  = 4,				// The system rnnning as a CAN frame sniffer			
	SYS_MODE_CONTEXT  = 8					// The system running for reading in-vehicle context		
} enumIpsMode;


struct sysRunningStatus {
	struct obdCanPortState	*can0_info;
	struct obdCanPortState	*can1_info;
	struct onboardPortState	*sensor_info;
	struct clientCommState	*comm_info;
	pthread_t								context_thread;
	pthread_t								onboard_thread;
	pthread_t								forward_thread;
	pthread_t								client_thread;
	bool										is_to_exit;
	uint										run_mode;
};




#define BREAK_IF_NOT_CONTEXT( sys_state ) \
	if(!((sys_state)->run_mode & SYS_MODE_CONTEXT)) \
		break;

#define RETURN_IF_NOT_CONTEXT( sys_state ) \
	if(!((sys_state)->run_mode & SYS_MODE_CONTEXT)) \
		return;

#define RETURN_IF_NOT_CONTEXT_RET( sys_state, ret ) \
	if(!((sys_state)->run_mode & SYS_MODE_CONTEXT)) \
		return ret;

#define CHECK_EXIT_BREAK( sys_state ) \
	if((sys_state)->is_to_exit) \
		break;

#define CHECK_EXIT_RETURN( sys_state ) \
	if((sys_state)->is_to_exit) \
		return;


void *threadRxCan0(void *arg);


#endif // __CAN_ITF_H
