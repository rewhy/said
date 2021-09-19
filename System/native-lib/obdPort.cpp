#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <typeinfo>
#include <linux/can.h>
#include <cutils/log.h>

#include "log.h"
#include "utils.h"


/* Wrapper for writing and reading frames */
int readFrame(uint can_fd, struct can_frame *frame) {
	int res = 0;
	int offset = 0;
	char buf[MAX_TMP_BUF]={0};
	// ALOGD("ReadFrame: To read frame from sk=%d", can_fd);
	// res = read(can_fd, frame, len);
	res = read(can_fd, frame, sizeof(struct can_frame));
	if(res > 0) {
		logRxFrame(frame);
		bytesToHexString(buf, frame->data ,frame->can_dlc);
		// ALOGD("LYY11 -----------------");
		ALOGD("ReadFrame: LYY11 Already Read %d(%d, %d) bytes from sk=%d (%s)", frame->can_dlc, sizeof(struct can_frame), res,  can_fd, buf);
	} else {
		ALOGE("ReadFrame: Already Read %d bytes from sk=%d (err=%d)", res,  can_fd, errno);
	}
	return res;
}

int writeFrame(uint can_fd, struct can_frame *frame, uint len) {
	int err = 0;
	char buf[MAX_TMP_BUF];

	bytesToHexString(buf, frame->data, frame->can_dlc);
	//ALOGD("LYY222 WriteFrame: To write frame of %d bytes to sk = %d (%s)", frame->can_dlc, can_fd, buf);
	err = write(can_fd, frame, len);
	logTxFrame(frame);
	//ALOGD("LYY222 WriteFrame: Aready written the frame to sk = %d (err=%d)", can_fd, err);

	return err;
}

int rxCanFrame(uint can_sk, struct can_frame *frame, uint us) {
	struct timeval wait_tv;
	fd_set rdfds;
	int nbytes, timeout;

	wait_tv.tv_sec  = us / 1000000;
	wait_tv.tv_usec = us % 1000000;

	FD_ZERO(&rdfds);
	FD_SET(can_sk, &rdfds);

	timeout = select(can_sk+1, &rdfds, NULL, NULL, &wait_tv);
	if(timeout < 0) {
		return -1;
	} else if (timeout == 0) {
		return 0;
	}
	// nbytes = read(can_sk, frame, sizeof(frame));
	nbytes = readFrame(can_sk, frame);
	return nbytes;
}

void txCanFrame(uint can_sk, uint id, uchar* data, uint length) {
	struct can_frame frame;
	char tmp[32];

	frame.can_id = id;
	frame.can_dlc = length;
	memcpy(frame.data, data, length);

	//write(can_sk, &frame, sizeof(frame));
	bytesToHexString(tmp, data, 8);
	ALOGD("To Tx (%d bytes): %s", length, tmp);
	writeFrame(can_sk, &frame, sizeof(frame));
}

