// isoTp.cpp

#include <stdio.h>
#include <stdlib.h>
#include <linux/can.h>
#include <sys/select.h>
#include <utils/Log.h>

#include "canitf.h"
#include "utils.h"
#include "log.h"
#include "isoTp.h"
#include "vehicle.h"
#include "obdPort.h"
#include "clientCom.h"
extern struct sysRunningStatus *sys_state;
/*
 * By default, the maximum payload of CAN message is 8 bytes, which is defined
 * in the standard ISO 15765-2 (ISO-TP). Meanwhile, the transfer protocol segments 
 * longer CAN message frame into multiple frame by adding some metadata named PCI 
 * (Protocol Control Information) that allows the interpretation of individual frames.
 * PCI can be 1, 2 or 3 bytes length and thus the data size of the frame is smaller.
 *
 * Get the type of the standard frame (defined by ISO-TP)
 * 1) Single Frame (SF)
 * 2) First Frame (FF)
 * 3) Consecutive Frame (CF)
 * 4) Flow Control Frame (FC): When receiving a FF frame, a FC frame is replied with
 * the information about status, block size, and seperation time.
 */ 
uint getFrameType(struct can_frame *frame) {
	unsigned int t = (frame->data[0] & PCI_FRAME_MASK) >> 4;
	if ( t > 3 ) {
		ALOGE("Unknow frame type (t=%d)!!!", t);
		return PCI_FRAME_IF;
	}
	return t;
}

uint getMulFrameSn(struct can_frame *frame) {
	if((frame->data[0] & PCI_FRAME_MASK) != PCI_FRAME_CF) {
		ALOGW("No SN in Frame type 0x%x", frame->data[0] & PCI_FRAME_MASK);
		return 0;
	}
	return frame->data[0] & 0x0f;
}

/* Get the length of the payload in the frame data */
uint getPayloadLength(struct can_frame *frame) {
	int l = 0;
	switch(frame->data[0] & PCI_FRAME_MASK) {
		case PCI_FRAME_SF:
			l = frame->data[0] & 0xff;
			break;
		case PCI_FRAME_FF:
			l = ((frame->data[0] & 0x0f) << 8) | frame->data[1];
			break;
		default:
			break;
	}
	return l;
}

static uint txSF(uint sk, ushort can_id, uchar* data, uint len) {
	uchar buff[8];
	memset(buff, 0, 8);
	buff[0] = 0x00; // SF
	buff[0] |= (0xF & len);
	memcpy(&buff[1], data, len);
	// txCanFrame(sk, can_id, buff, len+1);
	txCanFrame(sk, can_id, buff, 8);
	return len; // No payload left
}

static uint txFF(uint sk, ushort can_id, uchar* data, uint payload_len) {
	uchar buff[8];
	buff[0] = 0x10; // FF
	buff[0] |= ((payload_len & 0xF00) >> 8);
	buff[1] = 0xFF & payload_len;
	memcpy(&buff[2], data, 6);
	txCanFrame(sk, can_id, buff, 8);
	return 6; // The left payload
}

static uint txCF(uint sk, ushort can_id, uchar sn, uchar* data, uint len) {
	uchar buff[8];
	uint  size = len >= 7 ? 7 : len;
	buff[0] = 0x20; // CF
	buff[0] |= (0xF & sn);
	memcpy(&buff[1], data, size);
	if(len<7){
		memset(&buff[1+len],0x00,7-len);
	}
	txCanFrame(sk, can_id, buff, 8);
//txCanFrame(sk, can_id, buff, size+1);
	return size; // The left payload
}

static uint txFC(uint sk, ushort can_id, uchar fs, uchar bs, uchar st_min) {
	uchar buff[8];
    memset(buff,0,sizeof(buff));
	buff[0] = 0x30; // FC
	buff[0] |= (0xF & fs);		// Flow State
	buff[1] = 0xFF & bs;			// Block Size
	buff[2] = 0xFF & st_min;	// Minimum speration time between consecutive frames to be noticed
	txCanFrame(sk, can_id, buff, 8);
	return 0;
}

/* Waint for the FC frame */
static uchar* waitFC(uint can_sk, ushort src_id, struct can_frame *frame) {
	uint nbytes = 0;
	struct timeval wait_tv;
	fd_set rdfds;
	int timeout;

	// Wait for 100 ms at most
	wait_tv.tv_sec  = 0; 
	wait_tv.tv_usec = 300 * 1000;

	while(true) {

		FD_ZERO(&rdfds);
		FD_SET(can_sk, &rdfds);
		timeout = select(can_sk+1, &rdfds, NULL, NULL, &wait_tv);
		if(timeout < 0) { // Error
			ALOGE("%s: Wait frame error!!!", __func__);
			continue;
		}

		if(timeout == 0) { // Timeout
			ALOGD("%s: Receive frame timeout!", __func__);
			break;
		}

		nbytes = readFrame(can_sk, frame);

		if(frame->can_id == src_id) {
			if(getFrameType(frame) == PCI_FRAME_FC) {
				return frame->data;
			}
		}
	}

	return NULL;
}

/* Transmit specified application layer payload */
uint txPayload(uint can_sk, ushort src_id, ushort dst_id, uchar* data, uint size) {
	uint left_bytes = size;
	uchar fs, bs, st, sn=1;
	struct can_frame frame;
	uchar *pbuf = data;

	/* Single Frame */
	if(size <= 7) {
		txSF(can_sk, dst_id, data, size);
		return size;
	}
	ALOGD("LYY123 pos1");
	/* Multi-Frame */
	txFF(can_sk, dst_id, data, size);
	left_bytes -= 6;
	pbuf += 6;
	
	ALOGD("LYY123 pos2");
	if(waitFC(can_sk, src_id, &frame) == NULL) {
		return 0; // Tx payload failture
	}
	fs = frame.data[0] & 0xF;
	bs = frame.data[1];
	st = frame.data[2];
	ALOGD("LYY123 pos3");
	while(left_bytes > 0) {
		if(left_bytes > 7) {
			txCF(can_sk, dst_id, sn++, pbuf, 7);
			left_bytes -= 7;
			pbuf += 7;
		} else {
			txCF(can_sk, dst_id, sn++, pbuf, left_bytes);
			left_bytes = 0;
			pbuf = NULL;
		}
	}
	// TODO: time unit of FC frame 
	ALOGD("LYY123 TX Payload: %s", data);
	
	return size-left_bytes;
}

/* Receive specified application layer payload */
uint rxPayload(uint can_sk, ushort src_id, ushort dst_id, uchar* buff, uint size, int (*frameHandler)(struct can_frame *, uint)) {
	int rcv_len = 0, cpy_len = 0, nbytes = 0, left_bytes = 0, sn = 0, next_cf_sn = 0;
	struct can_frame frame;
	struct timeval wait_tv;
	fd_set rdfds;
	int timeout; 
	int i = 0;

	// Wait for 100 ms at most
	wait_tv.tv_sec  = 0; 
	wait_tv.tv_usec = 20 * 1000;


	while((sys_state->run_mode & SYS_MODE_CONTEXT) && (i < 5)) {

		FD_ZERO(&rdfds);
		FD_SET(can_sk, &rdfds);
		timeout = select(can_sk+1, &rdfds, NULL, NULL, &wait_tv); // TODO: using epoll() may improve performance

		if(timeout < 0) { // Error
			ALOGE("%s: Wait frame error!!!", __func__);
			continue;
		} 

		if(timeout == 0) { // Timeout without any frame received
			// ALOGW("Receive frame timeout!! %d %llu", i, getCurrentTime());
			i++;
			continue;
		}

		// Read the received frame from can0 port
		nbytes = readFrame(can_sk, &frame);

		if (nbytes < 0) {
			ALOGE("%s: Read raw socket error!!!", __func__);
			continue;
		}

		/* Paranoid check ... */
		if (nbytes < (int)sizeof(struct can_frame)) {
			ALOGE("%s: Read incomplete CAN frame!!!", __func__);
			continue;
		}

		/* Further process the received CAN frame */
		// printBuffer("CAN0: read:", frame.data, frame.can_dlc);

		/* If received from the target CAN_ID */
		if(frame.can_id != src_id) {
			ALOGW("%s: Not receive from the target can id (frame->can_id=%d)!!", __func__, frame.can_id);
			continue;
		}

		// GEt the type of the received frame
		uint frame_type   = getFrameType(&frame);
		uint payload_size = getPayloadLength(&frame);

		// TODO: Notice the client
		// Invoke the canback handler to further process the received frame
		if(frameHandler) {
			if ((*frameHandler)(&frame, payload_size) == -1)
				break;
		}

		/* Further change the session state */
		if(frame_type == PCI_FRAME_SF) {
			cpy_len = payload_size > size ? size : payload_size;
			memcpy(buff, &frame.data[1], cpy_len);
			rcv_len = cpy_len;
			break;
		} else if(frame_type == PCI_FRAME_FF) {

			/* If it is the first frame of the multiple (MUL) frames, we need			 
			 * to send a flow control frame for retrieving the next consecutive frames,
			 * otherwise the ECU will not send the consecutive frames.
			 *
			 * TODO: 
			 * 1) To ensure symchronization 
			 * 2) If the frame is triggered by the requests from CAN0, what need to be done?
			 */

			// Send FC frame to retrive all the consecutive frames
			txFC(can_sk, dst_id, 0, 0, 0);

			left_bytes = payload_size;
			cpy_len    = size > 6 ? 6 : size;
			memcpy(buff, &frame.data[2], cpy_len);
			rcv_len = cpy_len;
			left_bytes -= 6;
			next_cf_sn	= 1;							// The next CF

		} else if (frame_type == PCI_FRAME_CF) { // Process received CF
			sn = frame.data[0] & 0xf;
			if (next_cf_sn != sn) {
				ALOGE("The CF is received in reorder (rcv=%d wanted=%d)!!!", sn, next_cf_sn);
				break;
			}

			next_cf_sn += 1; // To receive next CF

			payload_size = left_bytes > 7 ? 7 : left_bytes;
			cpy_len = payload_size > (size - rcv_len) ? (size - rcv_len) : payload_size;
			memcpy(&buff[rcv_len], &frame.data[1], cpy_len);
			rcv_len += cpy_len;
			left_bytes -= payload_size;
			if(left_bytes == 0)  // All payload is received
				break;

		} else if(frame_type == PCI_FRAME_FC) { // FC frame
			ALOGE("The FC frame should not be received!!!");
			break;
		} else {
			ALOGE("Reserved frame should not be received (type=%d)!!!", frame_type);
			break;
		}
	}
	/* if(rcv_len > 0) {
		ALOGD("RX Payload: %s", buff);
	}  */
	return rcv_len;
}



/* =============================================== */
/* ======= Special CAN Response Parsing ========== */
/* =============================================== */



/*================ Frame Forwarding Functions ==========================*/
int forwardFrame(uint src_sk, uint dst_sk, int(*frameHandler)(struct can_frame *)) {
	struct can_frame frame;
	struct timeval wait_tv;
	fd_set rdfds;
	int nbytes, timeout, ret = 0, err = 0;
	ullong readTime = 0,writeTime = 0,diff = 0,readTime2 = 0,diff2 = 0 ;

	while(true) {
		// Wait for 600 ms at most
		wait_tv.tv_sec	= 0; 
		wait_tv.tv_usec = 800 * 1000;
		FD_ZERO(&rdfds);
		FD_SET(src_sk, &rdfds);
		
		timeout = select(src_sk+1, &rdfds, NULL, NULL, &wait_tv);

        //ALOGD("LYY427 POSTION1 src_sk:%d",src_sk);

		if(timeout < 0) { // Error
			ALOGE("LYY222 %s: Wait frame error!!!", __func__);
			break;
		}

		if(timeout == 0) { // Timeout
			//ALOGD("LYY427 %s: Receive frame timeout!", __func__);
			break;
		}

		nbytes = readFrame(src_sk, &frame);

		if(frame.can_id == 0x777){
			readTime = getCurrentTime();
		}
		//ALOGD("LYY427 DATA");
		if( nbytes < 0 ) {
			ALOGE("LYY222 %s: Read CAN frame error (%d) !!!", __func__, errno);
			continue;
		}
		
		
		if((uint)frame.can_id & 0xfffff000) {
			break;
		}
		/* Paranoid check ... */
		if (nbytes < (int)sizeof(struct can_frame)) {
			ALOGE("LYY427 %s: Read incomplete CAN frame!!", __func__);
			continue;
		}
		/* Further process the received CAN frame */
		//ALOGD("LYY222 Forward:0x%08x read: %d", frame.data, frame.can_dlc);

		// Invoke the canback handler to further process the received frame
		if(frameHandler) {
			ret = (*frameHandler)(&frame);
		}
		//ALOGD("LYY222 readFrame");
		ALOGD("LYY427 result: %d",ret);

		if(ret > 0) { // filter
			continue;
		} else if(ret < 0) { 
			break; // To stop forwarding
		} else {
            //ALOGD("LYY222 Forward the msg");
			/* Forward the frame */
			err = writeFrame(dst_sk, &frame, sizeof(frame));

			if(err < 0) {
				ALOGE("LYY222 %s: Tx frame error!!!", __func__);
			}
			if(frame.can_id == 0x777){
				writeTime = getCurrentTime();
				diff = writeTime-readTime;
				ALOGD("LYY428 %llu,%llu,DIFF:%llu",readTime,writeTime,diff);
			}
		}
	} // While
	return 0;
}
