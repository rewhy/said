// isoTp.h
#ifndef __ISO_TP_H
#define __ISO_TP_H


#include "utils.h"

#define MAX_LOAD_SIZE		128
#define MAX_ACK_SIZE		8
#define MAX_FRAME_SIZE	8

/* Contain the information of a special CAN frame */
struct ivCanFrameInfo {
	uint	id;
	uchar	data[MAX_FRAME_SIZE];
	uint	length;
};

#if 0
/* Contain the information of a special CAN response */
struct ivCanFrameInfo {
	unsigned int	id;
	unsigned char data[8];
	unsigned int	length;
};
#endif


#define PCI_FRAME_MASK	0xF0
typedef enum
{
	PCI_FRAME_SF = 0,	 /* The single frame */ 
	PCI_FRAME_FF = 1,	 /* The first frame */ 
	PCI_FRAME_CF = 2,   /* The continusous frame */
	PCI_FRAME_FC = 3,   /* The flow control frame */
	PCI_FRAME_IF = 4    /* The invalid frame */
} enum_PCI_FRAME;


uint getFrameType(struct can_frame *frame);
uint getPayloadLength(struct can_frame *frame);
uint getMulFrameSn(struct can_frame *frame);


unsigned int quitSession(ushort id);
void quitCurrentSession(struct IVSysInfo *iv_sys);
bool buildNewSession(ushort req_id, ushort ack_id, struct IVSysInfo *iv_sys);

uint txPayload(uint can_sk, ushort src_id, ushort dst_id, uchar* data, uint size);
//uint rxPayload(uint can_sk, ushort src_id, uchar* buff, uint size);
uint rxPayload(uint can_sk, ushort src_id, ushort dst_id, uchar* buff, uint size, int (*frameHandler)(struct can_frame *, uint));
void txCanFrame(uint can_sk, uint id, uchar* data, uint length);

// Forwarding
int forwardFrame(uint src_sk, uint dst_sk, int(*frameHandler)(struct can_frame *));

#endif // __ISO_TP_H
