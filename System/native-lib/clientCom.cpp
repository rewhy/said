// clientCom.cpp

#include <stdlib.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <sys/un.h>
#include <utils/Log.h>

#include "utils.h"
#include "log.h"
#include "vehicle.h"
#include "clientCom.h"
#include "obdPort.h"
#include "portControl.h"
#include "canitf.h"
// #include "localSocket.h"

#define SOCKET_NAME "canitfsocket"

extern struct sysRunningStatus *sys_state;

/*======================= Rx Client Message Functions =========================*/

/* Process the data received from the client */
static void processClientMsg(unsigned char *buff, int length){
	int canId = 0, dataLen = 0, dataLen1 = buff[3];
	uchar cmd	= buff[4], decesion = 0, action = 0, subcmd = 0;
	uchar data[256];
	char  t_buf[MAX_TMP_BUF];
	struct timeval tv;

	bytesToHexString(t_buf, buff, length);
	ALOGD("LYY111 Local Socket read buff (%d bytes) from APP: %s", length, t_buf);
	//ALOGD("LYY123 read app content");

	// Verify the message prefix
	if(buff[0] != MAGIC_SPLIT_1 || buff[1] != MAGIC_SPLIT_2){
		ALOGE("%s: The message head is wrong!!!", __func__);
		return;
	}

	if(length < 7){
		ALOGE("%s: The message is too small (%d, %s)!!!", __func__, length, t_buf);
		return;
	}

	switch(cmd){
		case CMD_FC_HEARTBEAT:	// Heartbeat Frame
			ALOGI("%s: HEART_BEAT", __func__);
			break;
		case CMD_FC_CAN_FRAME_DATA:	// CAN Frame
			ALOGI("%s: CAN FRAME DATA", __func__);
			if(length > 24){ // TODO: Why?
				return;
			}
			decesion = buff[5];
			switch(decesion) {
				case 0x00:
				case 0x01: // Just transmit the frame from CAN0 port
					dataLen = length-5-1-4-2;
					canId = arrayToInt(&buff[6]); // buff[6] | buff[7] << 8 | buff[8] << 16 | buff[9] << 24;
					memcpy(data, &buff[10], dataLen);

					usleep(2 * 1000);				
					//can0Send2(canId, data, dataLen);

					break;
				default:
					break;
			}
			break;
		case CMD_FC_GET_CAN_FILTER:	// get can filter id.
			break;
		case CMD_FC_SET_CAN_FILTER:	// set can filter id.
			break;
		case CMD_FC_ACTION_ID:			// Action
			subcmd = buff[5];
			doControlAction(subcmd, (uchar *)&data[6]);
			ALOGI("LYY111 %s: FC ACTION", __func__);
			break;
		default:
			break;
	}
}

/*======================= Tx Client Message Functions =========================*/

int txClientDecision(uchar *buff,uint length){
	int res, tx_len = 0;
	uchar load[512];
	if(sys_state->comm_info == NULL)
		return -1;
	if(sys_state->comm_info->com_sk <= 0)
		return -1;
	
	if(buff == NULL || length > 500) {
		ALOGE("Client: Do not support so big or null data!!!");
		return -1;
	}

	load[tx_len++] = MAGIC_SPLIT_1;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = TO_CLIENT_FILTER_INFO;
	load[tx_len++] = length;
	memcpy(&load[tx_len], buff, length);
	tx_len += length;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = MAGIC_SPLIT_1;

	pthread_mutex_lock(&sys_state->comm_info->com_lock);
	res = send(sys_state->comm_info->com_sk, load, tx_len, 0);
	pthread_mutex_unlock(&sys_state->comm_info->com_lock);
	
	return res;
}

int txClientSensorData(uchar* buff, uint length) {
	int res = 0, tx_len = 0;
	
	uchar load[512];

	if(!buff || length == 0 || length > 500) {
		ALOGE("Client: Do not support so big or null data!!!");
		return 0;
	}
	
	if(sys_state->comm_info == NULL)
		return -1;
	
	if(sys_state->comm_info->com_sk <= 0)
		return -1;

	load[tx_len++] = MAGIC_SPLIT_1;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = TO_CLIENT_ONBOARD_INFO;
	load[tx_len++] = length;
	// load[tx_len++] = 0;
	memcpy(&load[tx_len], buff, length);
	tx_len += length;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = MAGIC_SPLIT_1;

	pthread_mutex_lock(&sys_state->comm_info->com_lock);
	res = send(sys_state->comm_info->com_sk, load, tx_len, 0);
	pthread_mutex_unlock(&sys_state->comm_info->com_lock);
	
	return res;
}

/* Send the received UDS payload to the app client */
int txCxtPayload(uchar* buff, uint length) {
	int res, tx_len = 0;
	uchar load[512];
	if(sys_state->comm_info == NULL)
		return -1;
	if(sys_state->comm_info->com_sk <= 0)
		return -1;
	
	if(buff == NULL || length > 500) {
		ALOGE("Client: Do not support so big or null data!!!");
		return -1;
	}

	load[tx_len++] = MAGIC_SPLIT_1;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = TO_CLIENT_CONTEXT_INFO;
	load[tx_len++] = length;
	memcpy(&load[tx_len], buff, length);
	tx_len += length;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = MAGIC_SPLIT_1;

	pthread_mutex_lock(&sys_state->comm_info->com_lock);
	res = send(sys_state->comm_info->com_sk, load, tx_len, 0);
	pthread_mutex_unlock(&sys_state->comm_info->com_lock);
	
	return res;
}

/* Send the received CAN frame to the app client */
int txClientCanFrame(uint portNum, uint seqId, uint canId, uchar* buff, uint length) {
	int res, tx_len = 0;
	uchar load[512];
	if(sys_state->comm_info == NULL)
		return -1;
	if(sys_state->comm_info->com_sk <= 0)
		return -1;
	
	if(buff == NULL || length > 500) {
		ALOGE("Client: Do not support so big or null data!!!");
		return -1;
	}

	load[tx_len++] = MAGIC_SPLIT_1;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = TO_CLIENT_CAN_FRAME;
	load[tx_len++] = 0x9+length;
	load[tx_len++] = portNum;
	intToArray((int) seqId, &load[tx_len]);
	tx_len += 4;
	intToArray((int) canId, &load[tx_len]);
	tx_len += 4;
	memcpy(&load[tx_len], buff, length);
	tx_len += length;
	load[tx_len++] = MAGIC_SPLIT_2;
	load[tx_len++] = MAGIC_SPLIT_1;

	pthread_mutex_lock(&sys_state->comm_info->com_lock);
	res = send(sys_state->comm_info->com_sk, load, tx_len, 0);
	pthread_mutex_unlock(&sys_state->comm_info->com_lock);
	
	return res;
}

void receiveClientCommand(void (*newClientHandler)()) {
	int listen_sk = -1, client_sk = -1, ret = -1;
	struct sockaddr_un peeraddr;
	socklen_t	socklen = sizeof(peeraddr);
	fd_set rdfds;
	struct timeval wait_tv;
	int timeout, len;
	uchar buff[255];
	
	// listen_sk = createLocalSocket();
	listen_sk = android_get_control_socket(SOCKET_NAME);
	if (listen_sk < 0) {
		ALOGE("Failed to get socket '" SOCKET_NAME "' errno:%d", errno);
		return;
	}

	ALOGD("Start listening on the socket interface (sk=%d).", listen_sk);
	
	ret = listen(listen_sk, 6);
	if(ret < 0) {
		ALOGE("Error on listening the client socket (%d)!!!", errno);
		// close(listen_sk);
		return;
	}

	client_sk = accept(listen_sk, (sockaddr *)&peeraddr, &socklen);
	if(client_sk < 0) {
		ALOGE("Error on accept client socket errno=%d!!!", errno);
		// close(listen_sk);
		return;
	}
	
	sys_state->comm_info->com_sk = client_sk;

	/* Invoke the hanler when there is new client collecting to the service */
	(*newClientHandler)();
		
	wait_tv.tv_sec  = 1;
	wait_tv.tv_usec = 0;		

	while (!sys_state->is_to_exit) {
#if 0	
		FD_ZERO(&rdfds);
		FD_SET(client_sk, &rdfds);
		
		timeout = select(client_sk+1, &rdfds, NULL, NULL, &wait_tv);
		
		if(timeout < 0) {
			ALOGE("Client: read message error!!!");
			break;
		} else if(timeout == 0) {
			ALOGD("Client: receive message timeout!!!");
			break;
		}
#endif
		len = recv(client_sk, buff, sizeof(buff), 0);
		
		if(len == -1) {
			ALOGE("Client: recv socket data failed!!!");
			break;
		}
		
		ALOGD("To process client command!");
		processClientMsg((uchar*)buff, len);
	}

	close(client_sk);
	// close(listen_sk);
	
	sys_state->comm_info->com_sk = -1;
}
