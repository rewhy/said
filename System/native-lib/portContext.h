// portContext.h
#ifndef __CAN_CONTEXT_H
#define __CAN_CONTEXT_H

#include "canitf.h"
#include "utils.h"
#include "clientCom.h"
#include "vehicle.h"


struct ivFloatState {
	ullong ts=0;
	double st=0;
};

struct ivIntState {
	ullong ts;
	int		 st;
};

struct vehicleRunState {
	struct ivFloatState		speed;
	struct ivFloatState		rpm;
	struct ivFloatState		steer;
};

struct recordData{
	int type;//0:can_frame 1:sensor
	ullong ts;
	char data[100];
	struct recordData *next;
};

extern struct vehicleRunState v_run_state;

/* ================ OBDII related ================== */
enum obdService { // ISO 15031-5
	OBD_SUPPORTED_PID_REQ_SID = 0x00,
	OBD_POWERTRAIN_DIAG_SID		= 0x01,
	OBD_POWERTRAIN_FREE_SID		= 0x02,
	OBD_EMISSION_DIAG_REQ_SID = 0x03,
	OBD_EMISSION_DIAG_RST_SID = 0x04,
	OBD_OXYGEN_RESULT_SID			= 0x05,
	OBD_ONBOARD_SPECIFY_SID   = 0x06,
	OBD_EMISSION_CUR_DTC_SID	= 0x07,
	OBD_ONBOARD_CONTROL_SID		= 0x08,
	OBD_VEHICLE_INFO_SID			= 0x09,
	OBD_EMISSION_PER_DTC_SID  = 0x0A
};

struct obdContextInfo {
	float		res[196];
	ullong	qts[196];
};

struct obdContextData {
	uint	sid;
	uint	pid;
	uint	res;
	uint	ts1;
	uint	ts2;
};

struct udsContextData {
	uint sid;
	uint pid;
	uint res;
	uint ts1;
	uint ts2;
};

typedef enum {
	SUPPORTED_PIDS		= 0x00,
	MONITOR_STATUS,
	FREEZE_DTC,
	FUEL_SYSTEM_STATUS,
	CALCULATED_ENGINE_LOAD,
	ENGINE_COOLANT_TEMPERATURE,
	SHORT_FUEL_TRIM1,
	LONG_FUEL_TRIM1,
	SHORT_FUEL_TRIM2,
	LONG_FUEL_TRIM2,
	FUEL_PRESSURE,
	INTAKE_MANIFOLD_ABSOLUTE_PRESSURE,
	ENGINE_RPM,
	VEHICLE_SPEED,
	TIMING_ADVANCE,
	INTAKE_AIR_TEMPERATURE,
	MAF_AIR_FLOW_RATE,
	THROTTLE_POSITION,
	COMMANDED_2ND_AIR_STATUS,					// 0x12
	OXYGEN_SENSORS_PRESENT_2,
	OXYGEN_SENSOR_1,
	OXYGEN_SENSOR_2,
	OXYGEN_SENSOR_3,
	OXYGEN_SENSOR_4,
	OXYGEN_SENSOR_5,
	OXYGEN_SENSOR_6,
	OXYGEN_SENSOR_7,
	OXYGEN_SENSOR_8,
	OBD_STANDARDS,										// 0x1C
	ORYGEN_SENSORS_PRESENT_4,
	AUXILIARY_INPUT_STATUS,
	RUNTIME_ENGINE_START,							// 0x1F
	SUPPORTED_PIDS_20,								// 0x20
	DISTANCE_TRAVELED_WITH_MIL,
	FUEL_RAIL_PRESSURE,
	FUEL_RAIL_GAUGE_PRESSURE,
	OXYGEN_SENSOR_1_FUEL_VOLTAGE,
	OXYGEN_SENSOR_2_FUEL_VOLTAGE,
	OXYGEN_SENSOR_3_FUEL_VOLTAGE,
	OXYGEN_SENSOR_4_FUEL_VOLTAGE,
	OXYGEN_SENSOR_5_FUEL_VOLTAGE,
	OXYGEN_SENSOR_6_FUEL_VOLTAGE,
	OXYGEN_SENSOR_7_FUEL_VOLTAGE,
	OXYGEN_SENSOR_8_FUEL_VOLTAGE,
	COMMANDED_EGR,
	EGR_ERROR,
	COMMANDED_EVAPORATIVE_PURGE,
	FUEL_TANK_LEVEL_INPUT,
	WARM_UP_SINCE_CODE_CLEARED,
	DISTANCE_TRAVELED_SINCE_CODE_CLEARED,
	EVAP_SYSTEM_VAPOR_PRESSURE,
	ABSOLUTE_BAROMETRIC_PRESSURE,
	OXYGEN_SENSOR_1_FUEL_CURRENT,
	OXYGEN_SENSOR_2_FUEL_CURRENT,
	OXYGEN_SENSOR_3_FUEL_CURRENT,
	OXYGEN_SENSOR_4_FUEL_CURRENT,
	OXYGEN_SENSOR_5_FUEL_CURRENT,
	OXYGEN_SENSOR_6_FUEL_CURRENT,
	OXYGEN_SENSOR_7_FUEL_CURRENT,
	OXYGEN_SENSOR_8_FUEL_CURRENT,
	CATALYS_TEMPERATURE_BANK1_SENSOR1,	// 0x3C
	CATALYS_TEMPERATURE_BANK2_SENSOR1,
	CATALYS_TEMPERATURE_BANK1_SENSOR2,
	CATALYS_TEMPERATURE_BANK2_SENSOR2,
	SUPPORTED_PIDS_40,									// 0x40
	MONITOR_STATUS_THIS_DRIVE_CYCLE
} service01PIDS;

/* ================ UDS Related ==================== */

/* Store the payload of a special context request */
struct cxtRequest {
	char	name[32];
	uchar	sid;	// Service ID
	uchar	payload[MAX_LOAD_SIZE];
	uint	len;
	struct cxtRequest *next;
};

/* Context response info */
struct cxtInfo {
	struct cxtRequest*	req;
	ullong	ts;
	uint	len;
	uchar	data[0];
};
#if 0
/* Store the information for trieving context revelant to a in-vehicle system */
struct ivSysInfo {
	char	 name[32];
	ushort request_id;		// Functional Request CAN ID
	ushort ack_id;   // Functional Response CAN ID
	struct ivCanFrameInfo build_session_request;
	struct ivCanFrameInfo leave_session_request;
	struct cxtRequest *req_list;
	struct ivSysInfo  *next;
};

#endif

void sendUDSRequests(void);
void readInVehicleContext(void);
int processRcvContextFrame(struct can_frame *frame);

/***************************************************/
// void putNewCxtData(struct cxtRequest *cxt_req, uchar *ack_data, uint len);
struct cxtInfo* getNewCxtData();

/***************************************************/
void quitCurrentSession(struct IVSysInfo *iv_sys);
bool buildNewSession(ushort req_id, ushort ack_id, struct IVSysInfo *iv_sys);
void collectIVContext(void);

void modelTest(char * logname);
int getTestPID(uchar *data);
bool parseIVCxtAck(struct IVCxtInfo* cxt_info,unsigned char* testResp,int len);
void notifyCxtInfo(ullong ts, ushort sid, ushort pid, double res);


void startContextMode(void);

#endif // __CAN_CONTEXT_H
