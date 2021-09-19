#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <sys/stat.h>

#include "canitf.h"

#include "log.h"
#include "utils.h"
#include "vehicle.h"

#include "clientCom.h"

#include "obdPort.h"
#include "isoTp.h"

#include "portContext.h"
#include "portForward.h"
#include "onboard.h"

#include "ivSpec.h"
#include "udp.h"

#define XUELEI 0

struct sysRunningStatus *sys_state;


//#ifndef IS_ACTION_MODE
/* The mainly major task depending on the CAN1 port */
void *threadExternalPort(void *art) { // CAN1
	int fd = -1;
	struct ifreq ifr;
	struct sockaddr_can addr;

	system("netcfg can1 down");
	usleep(100 * 1000); // 100 ms
	system("ip link set can1 type can bitrate 500000 triple-sampling off");
	usleep(100 * 1000);
	system("netcfg can1 up");

	ALOGI("Start the external port thread.");

	/* Initialize the CAN1 state structure */
	sys_state->can1_info = (struct obdCanPortState*)malloc(sizeof(struct obdCanPortState));



	if(sys_state->can1_info == NULL) {
		free(sys_state->can1_info);
		ALOGE("Malloc error 2!!!\n");
		return 0;
	}

	sys_state->can1_info->obd_port_state = CAN_STATE_WILD; // Useless (WILD_STATE)

	fd = socket( PF_CAN, SOCK_RAW, CAN_RAW );
	if(fd == -1) {
		ALOGE("SocketCAN: Error opeing socket!!!");
		free(sys_state->can1_info);
		sys_state->can1_info = NULL;
		return NULL;
	}

	// Locate the interface to be used (aka, can0)
	strcpy(ifr.ifr_name, "can1");
	ioctl(fd, SIOCGIFINDEX, &ifr);

	// TODO: For setting filter for this connection 

	/* Bind the socket to can0 interface  */
	ioctl(fd, SIOCGIFINDEX, &ifr);
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		ALOGD("Bind can device failed\n");
		close(fd);
		sys_state->can1_info = NULL;
		return NULL;
	}
	/* Initialize CAN0 lock  */
	pthread_mutex_init(&sys_state->can1_info->can_lock, NULL);

	/* Read to work */
	sys_state->can1_info->obd_port_state = OBD_STATE_IDLE;
	sys_state->can1_info->can_sk = fd;

	ALOGD("LYY427 can1 UP %d\n",fd);
	/* The major forward task */
	while (!sys_state->is_to_exit) {
		// ALOGD("To forward");
		externalPortForward();
		//ALOGD("Fini forward");
	}

	ALOGI("To exit the external port thread.");

	/* To exit the th thread */
	close(fd);
	pthread_mutex_destroy(&sys_state->can1_info->can_lock);
	free(sys_state->can1_info);

	// TODO: For setting filter for this connection 
	return NULL;
}
//#endif


/* The thread for retrieving in-vehicle context information */
void *threadInternalPort(void *arg) { // Woring on CAN0 port mainly
	int fd = -1;
	struct ifreq ifr;
	struct sockaddr_can addr;

	ALOGD("Start the internal port thread (mode=%d).", sys_state->run_mode);

	system("netcfg can0 down");
	usleep(100 * 1000); // 100 ms
	system("ip link set can0 type can bitrate 500000 triple-sampling off");
	usleep(100 * 1000);
	system("netcfg can0 up");

	sys_state->can0_info = (struct obdCanPortState*)malloc(sizeof(struct obdCanPortState));
	if(sys_state->can0_info == NULL) {
		free(sys_state);
		ALOGE("Malloc error 1!!!\n");
		return NULL;
	}

	/* Is can0 ready for Rx/Tx */
	sys_state->can0_info->obd_port_state = CAN_STATE_WILD; // Useless (WILD STATE)

	fd = socket( PF_CAN, SOCK_RAW, CAN_RAW );
	if(fd == -1) {
		ALOGE("SocketCAN: Error opeing socket!!!");
		free(sys_state->can0_info);
		return NULL;
	}

	// Locate the interface to be used (aka, can0)
	strcpy(ifr.ifr_name, "can0");
	ioctl(fd, SIOCGIFINDEX, &ifr);

	// TODO: For setting filter for this connection 

	/* Bind the socket to can0 interface  */
	ioctl(fd, SIOCGIFINDEX, &ifr);
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		ALOGD("Bind can device failed\n");
		close(fd);
		free(sys_state->can0_info);
		sys_state->can0_info = NULL;
		return NULL;
	}

	/* Initialize com lock  */
	pthread_mutex_init(&sys_state->can0_info->can_lock, NULL);

	/* Read to work */
	sys_state->can0_info->obd_port_state = OBD_STATE_IDLE;
	sys_state->can0_info->can_sk = fd;

	/* Start to work */
	while (!sys_state->is_to_exit) {
		if(sys_state->run_mode == SYS_MODE_IDLE) { // Running idle mode (without any task)
			usleep(100 * 1000); // wait 100 ms
			continue;
		}
		if(sys_state->can0_info->obd_port_state != OBD_STATE_WILD) {
			if(sys_state->run_mode & SYS_MODE_CONTEXT) {
				/* To retrieve the IV context informaiton */
#if TEST
                
#else
				collectIVContext();
#endif
				//ALOGD("Start to retrieve the in-vehicle context!");				
			} else if (sys_state->run_mode & SYS_MODE_ACTION) {
				sleep(1);
			} else if (sys_state->run_mode & SYS_MODE_FORWARD) {
				ALOGD("Enter Forward mode!");				
				internalPortForward();
            } else {
                ALOGE("Unknown running mode!!!");
            }

		} else {
			usleep(1000); // wait 1 ms
		}
	}
	ALOGI("To exit the internal port thread.");
	/* To exit the th thread */
	close(fd);
	pthread_mutex_destroy(&sys_state->can0_info->can_lock);
	free(sys_state->can0_info);

	return NULL;
}

//#ifndef IS_ACTION_MODE
void* threadOnboard(void *art) {
	int accel_fd;
	sys_state->sensor_info = (struct onboardPortState*)malloc(sizeof(struct onboardPortState));
	if(sys_state->sensor_info == NULL) {
		ALOGE("Malloc sensor state structure failture!!!");
		return 0;
	}

	accel_fd = open(ONBOARD_SENSOR_NAME, O_RDONLY);
	if (accel_fd < 0) {
		ALOGE("Can not open device %s", ONBOARD_SENSOR_NAME);
		return NULL;
	}
	sys_state->sensor_info->acc_fd = accel_fd;
#if TEST
	ALOGD("LYY1111 Enter TEST");
	receive_UDP();
	//modelTest("/sdcard/test1.log");
	//modelTest("/sdcard/test2.log");

#else
	sleep(10);
	ALOGD("LYY start Sensor");
	startReadOnboardSensor();
#endif
	close(accel_fd);
	free(sys_state->sensor_info);
	sys_state->sensor_info = NULL;

	return NULL;
}
//#endif


/* If there is new client connecting to this canitfservice through local socket,
 * four threads are created to sniff the frames from can0 and can1 interfaces, 
 * read in-vehicle information, and read sensor data, respectively. */
void onNewClient(void) {
	ALOGD("New connection from client is accepted.");
}

/* Initialize the connection to the client */
void* threadClient(void *arg) {
	void (*ptr)() = &onNewClient;

	sys_state->comm_info = (struct clientCommState*)malloc(sizeof(struct clientCommState));
	if(!sys_state->comm_info) {
		ALOGE("Malloc client state structure error!!!");
		return NULL;
	}

	pthread_mutex_init(&sys_state->comm_info->com_lock, NULL);

	ALOGD("Start to listen command from client");

	while( !sys_state->is_to_exit ) {
		ALOGD("To listen new client");
		sys_state->comm_info->com_sk = -1;
		receiveClientCommand(ptr);
		usleep(100 * 1000);
	}

	pthread_mutex_destroy(&sys_state->comm_info->com_lock);
	free(sys_state->comm_info);
	sys_state->comm_info = NULL;
	return NULL;
}

static void startOBDIPS() {

	int num = get_pids_by_name(NULL, 2, "canitf");

	if (num > 0) {
		ALOGD("canitf exit as only 1 process allowed.\n");
		return;
	}

	/* 0: No client */
	sys_state->is_to_exit = false;

	/* 1: To start CONTEXT thread */
	pthread_create(&sys_state->context_thread,  NULL, threadInternalPort, NULL);
//#ifndef IS_ACTION_MODE
	/* 2: To start Forward thread */
	pthread_create(&sys_state->forward_thread,  NULL, threadExternalPort, NULL);
	/* 3: To start Onboard thread */
	pthread_create(&sys_state->onboard_thread,  NULL, threadOnboard, NULL);
//#endif
	/* 4: Wait for commands from clinet */
	pthread_create(&sys_state->client_thread,  NULL, threadClient, NULL);

	// pthread_join( sys_state->context_thread, NULL );
	// pthread_join( sys_state->forward_thread, NULL );
	// pthread_join( sys_state->onboard_thread, NULL );
}


// signo == 3
void sigQuitHandler(int signo) {

	ALOGD("Received QUIT signal (sigNo=%d).", signo);

	if(sys_state) {
		sys_state->is_to_exit = true;
		return;
	} else {
		exit(0);
	}
}

static void sigHandler(int sig, siginfo_t *siginfo, void *context) {
	pid_t sender_pid = siginfo->si_pid;
	sigQuitHandler(sig);
}

int main(int argc, char *argv[]) {
	static struct sigaction siga;
	int num = get_pids_by_name(NULL, 2, "canitf");
	ALOGD("CanIPS version: 0.10\n");

	if(initLog() == 0) 
		return 0;

	if (num > 0) {
		ALOGE("canitf exit as only 1 process allowed.\n");
		return 0;
	}

	/* Initialize port states (can0, can1, comm) */
	sys_state = (struct sysRunningStatus*)malloc(sizeof(struct  sysRunningStatus));
	if(sys_state == NULL) {
		ALOGE("Malloc error 0!!!\n");
		return 0;
	}
	memset(sys_state, 0, sizeof(struct sysRunningStatus));

	siga.sa_sigaction = *sigHandler;
	siga.sa_flags |= SA_SIGINFO;
	if(sigaction(SIGQUIT, &siga, NULL) != 0) {
		ALOGE("Error register sigaction for SIGQUIT");
		return 0;
	}
	// signal(SIGQUIT, sigQuitHandler);



	initUDSInfo();
	initControlInfo();

	initFormatRule();
#ifdef IS_ACTION_MODE
	//initControlInfo();
#else
	initFilterHookTable();
	// The ips just performs CONTEXT reading tasks
	startContextMode();
#endif
	startOBDIPS();


	while(!sys_state->is_to_exit) {
		sleep(1);
		ALOGD("Heartbeat");
	}

	ALOGD("To exit client listening thread.");
	exit(0);



	exitLog();
	freeFilterHookTable();

	return 0;
}
