// onboard.h
#ifndef __ON_BOARD_SENSOR_H
#define __ON_BOARD_SENSOR_H

#include <pthread.h>

#include "utils.h"

#define ACC_SYSFS_ENABLE	"enable"
#define ACC_DATA_NAME			"MPU6050-accel"
#define ONBOARD_SENSOR_NAME		"/dev/mpu6050_accel"

#define SCALE_FACTOR			(1000000.0f)

#define GRAVITY_EARTH			(9.80665f)
#define ACC_DATA_CONVERSION(value)	(((((double)value) * GRAVITY_EARTH) / ((double)0x4000)) * SCALE_FACTOR)
#define GYRO_DATA_CONVERSION(value) ((((double)value) * ((double)M_PI) / (180.0f)) * SCALE_FACTOR)

/* The state of the Sessor port */
struct onboardPortState {
	int							acc_fd;
	pthread_t				read_thread;
	pthread_mutex_t	sensor_lock;
};

struct axis_data {
	int x;
	int y;
	int z;
	int rx;
	int ry;
	int rz;
};

/*struct axis_data {
	double x;
	double y;
	double z;
	double rx;
	double ry;
	double rz;
};*/

// void *threadReadSensor(void *arg);
void startReadOnboardSensor(void);

#endif // __ON_BOARD_SENSOR_H
