
#ifndef CANBOX_H_
#define CANBOX_H_

#include <pthread.h>

#include "utils.h"

/* The current CAN IPS running states  and configuration */
typedef enum {
	CAN_STATE_WILD		= 0,				// The port is not initialized
	CAN_STATE_BOND		= 1,				// The port is already bond with CAN socket
	CAN_STATE_IDLE		= 2,				// The port is in idle state (neither sniffering nor session)
	CAN_STATE_SNIFF		= 3,				// The port is in sniffing state (just monirting frames)
	CAN_STATE_SESSION	= 4					// The port mantains a session with a in-vehicle system
} enumCanState;


/* The state of the current OBD port */
typedef enum {
	OBD_STATE_WILD	 = 0,		// No CAN socket bond
	OBD_STATE_RX		 = 1,		// Being receiving frames
	OBD_STATE_TX		 = 2,		// Being transmiting frames
	OBD_STATE_HAND	 = 3,   // Being handling (building session)
	OBD_STATE_IDLE	 = 4		// Idle
} enumPortState;

// #define MAX_PAYLOAD_SIZE		128
/* The states of the CAN port */
struct obdCanPortState {
	uint		can_sk;								// The CAN socket bond to this port
	uchar		obd_port_state;				// The port states
	ushort	can_session_id;				// The session states
	ushort	can_dst_id;						// The request CAN ID of current session
	ushort	can_src_id;						// The response CAN ID of current session
#if 0
	uint		in_vehicle_sys;

	/* For maintain the information about one request/respons */
	ushort	next_cf_sn;				    // Used for receving CF frame
	ushort  rx_payload_total_size;
	ushort  rx_payload_received_size;
	uchar		rx_buff[MAX_PAYLOAD_SIZE];
	uint		rx_buff_size;
	uchar		tx_buff[MAX_PAYLOAD_SIZE];
	uint		tx_buff_size;
	
	ulong		last_tx_ts;						// The timestamp of last trasmitted CAN frame
	ulong		last_rx_ts;						// The timsstamp of last received CAN frame
	ushort	last_tx_can_id;       // The CAN ID of the last received CAN frame
	ushort	last_rx_can_id;       // The CAN iD of the last transmitted CAN frame
	ushort  next_rx_can_id;
#endif
	uint		tx_frame_num;					// The number of transmitted CAN frames
	uint		rx_frame_num;					// The number of received can frames
	pthread_t				tx_thread;
	pthread_t				rx_thread;
	pthread_mutex_t	can_lock;			// The lock for packet Tx
};

/* The session states for further in-vehicle context retrieving */
typedef enum
{
	CAN_SESSION_IDLE  = 0,   /* There is no CAN session */
	CAN_SESSION_PORT0 = 1,   /* CAN0 maintains the session */
	CAN_SESSION_PORT1 = 2    /* CAN1 maittains the session */
} enum_CAN_PORT_STATE;

#if 0
void CanBoxObdInit(void);
void MulDelay(int i);
void WaitSuccess(int i);
bool waitCanResponse(uint can_id, uint timeout);

void sendUDSSessionRequest(unsigned int id);
void sendUDSDataRequest(unsigned int id, BYTE DIDH, BYTE DIDL);
void sendUDSQuitRequest(unsigned int id);
void sendUDSBeatRequest();

void sendFCFrame(uchar bs);


// void sendUDSRequests(void);


/*======== Tx CAN Frame =======*/
void can0Send(struct can_frame* frame);
void can0Send2(unsigned int id, unsigned char* data, unsigned int length);
void can1Send(struct can_frame* frame);
void can1Send2(unsigned int id, unsigned char* data, unsigned int length);

/*======== Others ==========*/
ushort getReqIDFromRep(ushort id);
ushort getRepIDFromReq(ushort id);
int	   getSubsysIndex(ushort id);
#endif
/*==========================*/
int readFrame(uint can_fd, struct can_frame *frame);
int writeFrame(uint can_fd, struct can_frame *frame, uint len);

int rxCanFrame(uint can_sk, struct can_frame *frame, uint us);
void txCanFrame(uint can_sk, uint id, uchar* data, uint length);

#endif /* CANBOX_H_ */
