// clientCom.h
#ifndef __CLIENT_COM_H
#define __CLIENT_COM_H

#include <sys/types.h>

#include "utils.h"

// The prefix and postfix of the local socket messages
#define MAGIC_SPLIT_1 0xFF
#define MAGIC_SPLIT_2 0xFE

// Client --> Service
typedef enum {
	TO_CLIENT_HEARTBEAT		 = 0x00,
	TO_CLIENT_CAN_FRAME		 = 0x01,
	TO_CLIENT_ONBOARD_INFO = 0x02,
	TO_CLIENT_CONTEXT_INFO = 0x03,
	TO_CLIENT_FILTER_INFO = 0x04
} enum_SERVICE_DATA_TYPE;
// #define TO_CLIENT_FILTER_ID					0x03

// Client <-- Service
#define FROM_SERVICE_CAN_FRAME				0x91
#define FROM_SERVICE_GET_CAN_FILTER		0x92
#define FROM_SERVICE_SET_CAN_FILTER		0x93
/* The command of the message between this process and the client (APP) */
typedef enum {
	/* For the message to client */
	CMD_TC_HEARTBEAT			= 0x00,
	CMD_TC_CAN_FRAME			= 0x01,
	CMD_TC_ONBOARD_INFO   = 0x02,
	CMD_TC_CONTEXT_INFO		= 0x03,
} enum_CLIENT_CMD;

typedef enum {
	/* For the message from client */
	CMD_FC_HEARTBEAT			= 0x11,
	CMD_FC_CAN_FRAME_DATA	= 0x12,
	CMD_FC_GET_CAN_FILTER = 0x13,
	CMD_FC_SET_CAN_FILTER = 0x14,
	CMD_FC_ACTION_ID			= 0x15
} enum_APP_CMD;

typedef enum {
    IV_CONTEXT_SPEED    = 0x01,
    IV_CONTEXT_RPM      = 0x02
} enum_APP_CONTEXT_INFO;



typedef enum {
	CMD_ACTION_HORN			= 0x21,
	CMD_ACTION_UNLOCK		= 0x22,
	CMD_ACTION_LOCK			= 0x23,
	CMD_ACTION_OPEN_WINDOW  = 0x24,
	CMD_ACTION_CLOSE_WINDOW	= 0x25,
	CMD_ACTION_CLOSE_RVM	= 0x26,
	CMD_ACTION_OPEN_RVM	 	= 0x27,
	
	CMD_ACTION_DIPPED_LIGHT = 0x28,
	CMD_ACTION_HIGH_BEAM    = 0x29,
	CMD_ACTION_TURN_SIGNAL_LAMP = 0x30,
	CMD_ACTION_FOGLIGHT     = 0x31,
	CMD_ACTION_REARFOGLAMP  = 0x32,
	CMD_ACTION_BRAKELAMP    = 0x33,
	CMD_ACTION_CENTRALLOCKLIGHT  = 0x34,
	CMD_ACTION_REVERSINGLAMP = 0x35,
	CMD_ACTION_DOME_COLCK   = 0x36,
	CMD_ACTION_WINDSCREEN_WASHERS = 0x37,
	CMD_ACTION_SIGNAL_HORN = 0x38,
	CMD_ACTION_FRONTDRIVERS_CLOSEWINDOW = 0x39,
	CMD_ACTION_FRONTDRIVERS_OPENWINDOW = 0x40,
	CMD_ACTION_REARDRIVERS_CLOSEWINDOW = 0x41,
	CMD_ACTION_REARDRIVERS_OPENWINDOW = 0x42,
	CMD_ACTION_FRONTPASSENGER_CLOSEWINDOW = 0x43,
	CMD_ACTION_FRONTPASSENGER_OPENWINDOW = 0x44,
	CMD_ACTION_REARPASSENGER_CLOSEWINDOW = 0x45,
	CMD_ACTION_REARPASSENGER_OPENWINDOW = 0x46,
	CMD_ACTION_LIGHTING = 0x47,
	CMD_ACTION_INDICATORLIGHT = 0x48
} enum_ACTION_SUBCMD;

/* The state of communication socket */
struct clientCommState {
	int			com_sk;								// The socket for communication with the client (app)
	pthread_mutex_t	com_lock;     // The lock for using the communication socket
};

struct comIVContextInfo{
  uint   cxt_type;
  float  value;
};

int txClientDecision(uchar *buff,uint length);
int txClientCanFrame(uint portNum, uint seqId, uint canId, uchar* buff, uint length);
int txCxtPayload(uchar* buff, uint length);
int txClientSensorData(uchar* buff, uint length);

void receiveClientCommand(void (*newClientHandler)());
#endif // __CLIENT_COM_H
