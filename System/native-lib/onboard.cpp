// onboard.c

#include <math.h>
#include <poll.h>
#include <fcntl.h>
#include <utils/Log.h>

#include "canitf.h"
#include "log.h"
#include "utils.h"
#include "clientCom.h"
#include "onboard.h"


extern struct sysRunningStatus *sys_state;

/* Enable write data to the sensor device */
static int sensorWriteEnable(int isEnable) {
	char attr[PATH_MAX] = { '\0' };

	strcpy(attr, "/sys/devices/virtual/input/input2");
	strcat(attr, "/");
	strcat(attr, ACC_SYSFS_ENABLE);

	int fd = open(attr, O_RDWR);
	if (0 > fd) {
		ALOGE("Could not open (write-only) SysFs attribute \"%s\" (%s).", attr,
				strerror(errno));
		return -errno;
	}

	char buf[2];

	if (isEnable) {
		buf[0] = '1';
	} else {
		buf[0] = '0';
	}
	buf[1] = '\0';

	int err = 0;
	err = write(fd, buf, sizeof(buf));

	if (0 > err) {
		err = -errno;
		ALOGE("Could not write SysFs attribute \"%s\" (%s).", attr,
				strerror(errno));
	} else {
		err = 0;
	}

	close(fd);
	ALOGD("writeEnable:%d success!", isEnable);
	return err;
}

/* Read sensor data from the device */
static int sensorRead() {
	char buf[256];
	int n, nread;
	double logdata[6];
	int16_t *data;
	int16_t axis;
	struct axis_data gy_axis;
	struct pollfd mPollFds[1];

	if (sys_state->sensor_info->acc_fd) {
		mPollFds[0].fd      = sys_state->sensor_info->acc_fd;
		mPollFds[0].events  = POLLIN;
		mPollFds[0].revents = 0;
		
		memset(buf, 0, sizeof(buf));
		
		/* wait for new event */
		n = poll(mPollFds, 1, 0);  // re-check if fifo is ready
		if (n <= 0) {
			//			mFifoCount = 0;
			ALOGE("readSensor quit!!!");
			return -EINVAL;
		}

		/* Read accel data from the device */
		nread = read(sys_state->sensor_info->acc_fd, buf, sizeof(buf));
		if (nread > 0) {
			data = (int16_t *)buf;
			gy_axis.x	 = ACC_DATA_CONVERSION(data[0]);
			gy_axis.y  = ACC_DATA_CONVERSION(data[1]);
			gy_axis.z  = ACC_DATA_CONVERSION(data[2]);

			gy_axis.rx = GYRO_DATA_CONVERSION(data[3]);
			gy_axis.ry = GYRO_DATA_CONVERSION(data[4]);
			gy_axis.rz = GYRO_DATA_CONVERSION(data[5]);

			logdata[0] = (double)gy_axis.x/SCALE_FACTOR;
			logdata[1] = (double)gy_axis.y/SCALE_FACTOR;
			logdata[2] = (double)gy_axis.z/SCALE_FACTOR;
			logdata[3] = (double)gy_axis.rx/SCALE_FACTOR;
			logdata[4] = (double)gy_axis.ry/SCALE_FACTOR;
			logdata[5] = (double)gy_axis.rz/SCALE_FACTOR;

			ALOGD("Onboard Info: axis x=%f,y=%f,z=%f,rx=%f,ry=%f,rz=%f", 
					(double)gy_axis.x/SCALE_FACTOR,  (double)gy_axis.y/SCALE_FACTOR, 
					(double)gy_axis.z/SCALE_FACTOR,  (double)gy_axis.rx/SCALE_FACTOR, 
					(double)gy_axis.ry/SCALE_FACTOR, (double)gy_axis.rz/SCALE_FACTOR);
			
			txClientSensorData((uchar*)&gy_axis, sizeof(gy_axis));

			logSensor(logdata);
		}
	}
	return 0;
}



/* Read data from on-board sensors (six metrics) iteratively */
void startReadOnboardSensor() {
	sensorWriteEnable(1);
	while (!sys_state->is_to_exit) {
		// usleep(50 * 1000);	//100ms.
		sensorRead();
		usleep(50*1000);
	}
	ALOGD("Leaving the sensor reading thread.");
}

#if 0
int startOnboardThread(void) {
	sys_state->sensor_info = (struct onboardPortState*)malloc(sizeof(struct onboardPortState));
	if(sys_state->sensor_info == NULL) {
		ALOGE("Malloc sensor state structure failture!!!");
		return 0;
	}
	pthread_create(&sys_state->onboard_thread,  NULL, threadReadSensor, NULL);
	return 1;
}
#endif
