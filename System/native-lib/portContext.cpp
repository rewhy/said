// portContext.c

#include <utils/Log.h>
#include <linux/can.h>

#include "canitf.h"
#include "utils.h"
#include "log.h"
#include "obdPort.h"
#include "isoTp.h"
#include "vehicle.h"
#include "clientCom.h"
#include "portContext.h"

#include "obdCxtStr.cpp"
#include "onboard.h"
#include "ivSpec.h"

extern struct sysRunningStatus *sys_state;

struct vehicleRunState v_run_state;

struct IVSysInfo	*iv_sys_list = NULL;


/* ============================================================= */
/* ======== The buff for storing the context information ======= */
/* ============================================================= */

// 64M bytes
#define CXT_BUF_SIZE		1024 * 1024 * 64

static uchar cxt_ring_buf[CXT_BUF_SIZE];
static uchar *r_head = cxt_ring_buf;
static uchar *r_tail = cxt_ring_buf;

/* =============================================================== 
 * The ring-buffer structure for storing the received Context info
 *
 * ~: Bytes can be used to store context
 * @: The context data in the ring-buffer
 *
 * |<--------------------- CXT_BUF_SIZE ------------------------>|
 * ~~~~~~~~~~~~~~~~~~~~~@@@@@@@@@@@@@@@@@@@@@@@@@@~~~~~~~~~~~~~~~~
 * ^                    ^                         ^
 * |                    |                         |
 * V                    V                         V
 * cxt_ring_buf      r_tail                     r_head 
 *
 * ============================================================= */

static struct cxtInfo* putNewCxtData(struct cxtRequest *cxt_req, uchar *ack_data, uint len) {
	struct cxtInfo *c_info = NULL, *tmp_info = NULL;
	uint size = sizeof(struct cxtInfo) + len;
	uint right_free_size = cxt_ring_buf + CXT_BUF_SIZE - r_head;
	if(right_free_size < size) {
		/* If no enough right room left for storing the context info, set
		 * the right room NULL and start to use the ring buff from its head.
		 */
		c_info = (struct cxtInfo*)r_head;
		memset(r_head, 0, right_free_size);
	} else {
		c_info = (struct cxtInfo*)cxt_ring_buf;
	}
	if((uchar*)c_info + size > r_tail) {
		ALOGW("Context generating is faster than consuming currently!!");
		tmp_info = (struct cxtInfo*)r_tail;
		/* Let r_tail point the next cxtInfo structure since the current one is destroyed */
		r_tail = r_tail + sizeof(struct cxtInfo) + tmp_info->len;
	}
	c_info->req = cxt_req;
	c_info->ts  = getCurrentTime();
	c_info->len = len;
	memcpy(c_info->data, ack_data, len);
	r_head = (uchar *)c_info + size; 
	return c_info;
}

struct cxtInfo* getNewCxtData() {
	return NULL;
}

/* =============================================== */
/* =============== OBD/ISO 15765-4 ================*/
/* =============================================== */

/* Retrieve in-vehicle information through OBD (ISO 15765-4) protocol */


#define IS_SUPPORT_PID(pids, idx)	((unsigned int)pids & (0x80000000>>idx))

static uint querySupportedPids(ushort req_sid, ushort rep_sid, uchar sid, uchar offset) {
	uchar obd_req_data[2];
	uchar obd_ack_data[8];
	char		tmp[128];
	uint  tx_len = 0, rx_len = 0, i = 0, j = 0;
	uint supported_pids;

	obd_req_data[0] = sid;
	obd_req_data[1] = offset;
	tx_len = txPayload(sys_state->can0_info->can_sk, rep_sid, req_sid, obd_req_data, 2);
	if(tx_len <= 0)
		return 0;

	rx_len = rxPayload(sys_state->can0_info->can_sk, rep_sid, req_sid ,obd_ack_data, 8, NULL);

	supported_pids = 0;

	if(rx_len >= 2 &&  obd_ack_data[0] == 0x41 && obd_ack_data[1] == offset) { // Successfull response
		for(j = 0; j < 4; j++) {
			for(i = 0; i < 8; i++) {
				if(((uchar)0x80 >> i) & obd_ack_data[j+2]) {
					supported_pids |= (((uint)0x80000000) >> (j * 8 + i));
				}
			}
		}
	} else {
		bytesToHexString(tmp, obd_ack_data, rx_len);
		ALOGE("Received error frame (%s)!!!", tmp);
		return 0;
	}
	return supported_pids;
}


void notifyCxtInfo(ullong ts, ushort sid, ushort pid, double res) {
	struct obdContextData obd_data;

	memset((void*)&obd_data, 0, sizeof(obd_data));
	obd_data.sid = (uint)sid;
	obd_data.pid = (uint)pid;
	//obd_data.res = (uint)(res * 1000.0f);
    int tmp;
    tmp=int(res*1000.0f+0.5);
    obd_data.res = (uint)tmp;
    ALOGD("LYY000 %f || 0x%08x ",res,obd_data.res);
	obd_data.ts1 = (uint)(ts >> 32);
	obd_data.ts2 = (uint)(ts & 0xffffffff);
	txCxtPayload((uchar*)&obd_data, sizeof(obd_data));
	ALOGD("Vehicle: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x", obd_data.sid,
			obd_data.pid, obd_data.res, obd_data.ts1, obd_data.ts2);
}



inline
static int retrieveOBDCxt(ushort req_cid, ushort ack_cid, uchar *tx_buf, uint len, uchar *rx_buf, uint rx_max_len) {
	int rx_len = 0, tx_len = 0;
	char tmp[128];

	tx_len = txPayload(sys_state->can0_info->can_sk, ack_cid, req_cid, tx_buf, len);
	if(tx_len <= 0) {
		return -1;
	}
	
	rx_len = rxPayload(sys_state->can0_info->can_sk, ack_cid, req_cid, rx_buf, rx_max_len, NULL);
	if(rx_len >= 2 &&  rx_buf[0] == 0x41 && tx_buf[1] == rx_buf[1]) { // Successfull response
		bytesToHexString(tmp, rx_buf, rx_len);
		ALOGD("OBD CXT (%d bytes): sid=%d %s", rx_len, rx_buf[1], tmp);
	} else {
		ALOGE("OBD CXT (%d bytes): sid=%d No valid response received", rx_len, tx_buf[1]);
	}
	return rx_len;
}

static int retrieveUDSCxt(ushort req_cid, ushort ack_cid, uchar *tx_buf, uint len, uchar *rx_buf, uint rx_max_len){
	int rx_len = 0, tx_len = 0;	
	char tmp[128];

	tx_len = txPayload(sys_state->can0_info->can_sk,ack_cid,req_cid,tx_buf,len);
	if(tx_len<=0){
		return -1;
	}

	rx_len = rxPayload(sys_state->can0_info->can_sk,ack_cid,req_cid,rx_buf,rx_max_len,NULL);
    //ALOGD("LYY3 rx_len:%d",rx_len);
	if(rx_len>=3 && rx_buf[0] == 0x62 && tx_buf[1]==rx_buf[1] && tx_buf[2]==rx_buf[2]){
		bytesToHexString(tmp, rx_buf, rx_len);
		ALOGD("UDS  Context: %s", tmp);
	}else{
		ALOGE("UDS  ERROR");
	}
	return rx_len;
}

static void retrieveOBDPIDInfo(struct obdContextInfo *obd_cxt, ushort req_cid, ushort ack_cid, uchar sid, uint s_pids, uchar offset) {
	
	uchar	obd_req_data[8];
	uchar	obd_ack_data[8];
	uchar pid = 0;

	char	tmp[128];
	uint  tx_len = 0, rx_len = 0;
	int   i_res = 0;
	double f_res = 0.0;
	
	for(int i = 0; i < 0x20; i++) {
		
		if(!IS_SUPPORT_PID(s_pids, i))
			continue;
		
		pid = i + offset + 1;
		
		obd_req_data[0] = sid;
		obd_req_data[1] = pid;

		switch(pid) {
			case INTAKE_MANIFOLD_ABSOLUTE_PRESSURE: // 0x0B
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
					obd_cxt->res[pid] = (float)obd_ack_data[2];
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case THROTTLE_POSITION:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
					//obd_cxt->res[pid] = (float)obd_ack_data[2] * 100.0f / 255.0f;
					obd_cxt->res[pid] = (float)obd_ack_data[2] * 100.0f / 100.0f;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case TIMING_ADVANCE:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
					obd_cxt->res[pid] = (float)obd_ack_data[2]/2.0f-64.0;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case CALCULATED_ENGINE_LOAD:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
				  obd_cxt->res[pid] = (float)obd_ack_data[2]*100.0f/255.0f;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case FUEL_TANK_LEVEL_INPUT:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
					obd_cxt->res[pid] = (float)obd_ack_data[2]*100.0f/255.0f;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case VEHICLE_SPEED:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
					obd_cxt->res[pid] = (float)obd_ack_data[2];
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					//update speed
					v_run_state.speed.st = (double)obd_cxt->res[pid];
					v_run_state.speed.ts = obd_cxt->qts[pid];
					ALOGD("OBD: %s is %f km/h", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case ENGINE_RPM:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 4) {
					obd_cxt->res[pid] = ((float)obd_ack_data[2] * 256 + (int)obd_ack_data[3])/4;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);

					//update rpm
					v_run_state.rpm.st = (double)obd_cxt->res[pid];
					v_run_state.rpm.ts = obd_cxt->qts[pid];
					ALOGD("OBD: %s is %f rpm", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case ENGINE_COOLANT_TEMPERATURE:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
					obd_cxt->res[pid] = (float)obd_ack_data[2] - 40;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f C", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case INTAKE_AIR_TEMPERATURE:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 3) {
					obd_cxt->res[pid] = (float)obd_ack_data[2] - 40;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f c", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			case MAF_AIR_FLOW_RATE:
				rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
				if(rx_len >= 4) {
					obd_cxt->res[pid] = ((float)obd_ack_data[2] * 255.0f + (float)obd_ack_data[3]) / 100.0f;
					obd_cxt->qts[pid] = getCurrentTime();
					notifyCxtInfo(obd_cxt->qts[pid], sid, pid, obd_cxt->res[pid]);
					ALOGD("OBD: %s is %f grams/sec", obd_cxt_str[pid], obd_cxt->res[pid]);
				} else {
					ALOGE("OBD: Received error response for %s", obd_cxt_str[pid]);
				}
				break;
			default:
				break;
		}
		usleep(50 * 1000);
	}
}

/* Retrieve the in-vehicle information specifed by the sevice id */
static void retrieveOBDInfo(uchar sid) {
	struct obdContextInfo obd_info; 
	ushort obd_req_id	= 0x7df;
	ushort obd_ack_id	= 0x7e8;

	uchar	obd_req_data[8];
	uchar	obd_ack_data[8];

	char tmp[128];
	uint tx_len = 0, rx_len = 0, s_pids = 0;

	memset((void*)&obd_info, 0, sizeof(obd_info));
	for(int o = 0; o < 2; o++) {
		uint offset = 0x20 * o;
		s_pids = querySupportedPids(obd_req_id, obd_ack_id, sid, offset);
		// ALOGD("ECU (sid=%0x) supports pids: 0x%08x", sid, s_pids);
		if(s_pids > 0) {
			retrieveOBDPIDInfo(&obd_info, obd_req_id, obd_ack_id, sid, s_pids, offset);
		}
	}
}


int getTestPID(uchar *data){
    if(data[2]==0xf4 && data[3]==0x0d){
        return 1;
    }
    if(data[2]==0x12 && data[3]==0xb0){
        return 2;
    }
    if(data[2]==0x10 && data[3]==0x5c){
        return 4;
    }
    if(data[2]==0xf4 && data[3]==0x0c){
        return 3;
    }
    if(data[2]==0x15 && data[3]==0xd1){
        return 5;
    }
    if(data[2]==0x16 && data[3]==0x33){
        return 6;
    }
    if(data[2]==0x15 && data[3]==0xcf){
        return 7;
    }
    if(data[2]==0x15 && data[3]==0xd0){
        return 8;
    }
    if(data[2]==0x38 && data[3]==0x03){
        return 9;
    }
    if(data[2]==0x38 && data[3]==0x04){
        return 10;
    }
    if(data[2]==0x18 && data[3]==0x12){
        return 11;
    }
    return 0;
}

void modelTest(char * logname){
    //LYYTEST
    char buff[255];
    char *field = NULL;//first head
    char *data = NULL;//NEXT PART data
    char type[3] = {0};//OB or RX
//Sensor
    ullong time =0;
    ullong lastTime = 0;
    ullong interval=0;

    float logdata[6]={0};
    struct axis_data gy_axis;
//CAN Frame
    uint can_id;
    uchar pid = 0;
    char payload[100]={0};
    uchar bytesData[8]={0};

    //struct recordData *newnode = NULL;
    //struct recordData *top=NULL,*tmp=NULL;
    if(logname == NULL){
        return ;
    }

    FILE *fp = fopen(logname,"r");
    if(!fp) {
		ALOGE("LYYTEST: Cannot open file %s", logname);
		return ;
	}
    
    ALOGD("LYY111 Start to read file");
    while(fgets(buff,255,fp)){
        
        //newnode = (struct recordData*)malloc(sizeof(struct recordData))
        //if(newnode == NULL) {
			//ALOGE("LYYTEST: Allocate new action node error!!!");
			//break;
		//}
        //memset((void*)newnode, 0, sizeof(struct recordData));

        field = strtok(buff,":");
        data = strtok(NULL,":");        
        //ALOGD("LYY111 %s",field);
        sscanf(field,"[%2s@%llu]",type,&time);
        //ALOGD("LYY111 pos1");
        if(strcmp(type,"OB")==0){
            //Sensor
            sscanf(data," %f, %f, %f, %f, %f, %f", &logdata[0],&logdata[1],&logdata[2],&logdata[3],&logdata[4],&logdata[5]);
            gy_axis.x = (int)(logdata[0]*SCALE_FACTOR);
            gy_axis.y = (int)(logdata[1]*SCALE_FACTOR);
            gy_axis.z = (int)(logdata[2]*SCALE_FACTOR);
            gy_axis.rx = (int)(logdata[3]*SCALE_FACTOR);
            gy_axis.ry = (int)(logdata[4]*SCALE_FACTOR);
            gy_axis.rz = (int)(logdata[5]*SCALE_FACTOR);

            txClientSensorData((uchar*)&gy_axis, sizeof(gy_axis));
            

        }
        else if(strcmp(type,"RX")==0){
			//ALOGD("LYYTEST start to read rx");
            //parse
            sscanf(data," 0x%03x, %s",&can_id,payload);
            int length = String2Bytes(bytesData,payload,8);

            if(bytesData[1]!=0x62){
                continue;
            }
            pid = getTestPID(bytesData);

            if(length>=7){
                parseIVCxtAck(ack_info,bytesData,8);
                notifyCxtInfo(time,0x22,pid,ack_info->result);
            }

        }else{
            continue;
        }

        //ALOGD("LYY111 pos2");
        //SLEEP
        if(lastTime==0){
            lastTime = time;
        }
        else{
            interval = time - lastTime;
            if(interval>=0){
                if(interval>1000000){
                    interval = 1000000;
                }
                usleep(interval);
            }else{
                break;
                ALOGE("LYYTEST: TIME IS ERROR!");
            }
            lastTime =time;

        }

    }

    fclose(fp);
}


//set UDS pid
//TODO
static int getUDSPID(char* name){
    
    //char *Used_name[] = {"Speed","Acc_Sword","RPM","Accelerator","Brake_Pressure","Lateral_Acc","Vertical_Acc","Vertical_Acc_Raw_Value","Tilt_Angular_Acc_Raw_Value","Auto_Trans_Brake_Pressure","Steering_Angle"};
    char *Used_name[] = {"Speed","Acc_Sword","RPM","Brake_Pressure","Lateral_Acc","Vertical_Acc","Vertical_Acc_Raw_Value","Tilt_Angular_Acc_Raw_Value","Auto_Trans_Brake_Pressure","Auto_Trans_Accelerator","Steering_Angle"};
	int i;
    for(i=0;i<11;i++){
        if(strcmp(name,Used_name[i])==0){
            return (i+1);
        }
    }

	return 0;
}

static bool retrieveUDSInfo(uchar sid,struct IVReqInfo *req_list_head){
	struct IVReqInfo *tmp_list = req_list_head;

	ushort uds_req_id;
	ushort uds_ack_id;

	uchar uds_req_data[8]={0};
	uchar uds_ack_data[16]={0};

	uchar all_ack_data[9]={0};
	
	uchar pid=0;
	int rx_len=0;

	ullong this_time;

	while(tmp_list && (sys_state->run_mode & SYS_MODE_CONTEXT)){
		//pid
		//set the frequence and condition
		if(tmp_list->flag == false){
			continue;
		}

		if(tmp_list->frequence>0){
			tmp_list->frequence = tmp_list->frequence-1;
			continue;
		}

		pid = getUDSPID(tmp_list->name); 
        
        ALOGD("LYY9 PID1:%d",pid);
		if(pid==0){
            ALOGD("LYY5 PID : %d",pid);
            tmp_list = tmp_list->next;
			continue;
		}

		memcpy(uds_req_data,tmp_list->req_data+1,tmp_list->req_len);

		uds_req_id = tmp_list->req_id;
		uds_ack_id = tmp_list->ack_id;
       //ALOGD("LYY2 REQID:%d",uds_req_id);

		rx_len = retrieveUDSCxt(uds_req_id,uds_ack_id,uds_req_data,tmp_list->req_len,uds_ack_data,16);
		
		//ALOGD("LYY3 %d",rx_len);
		
        if(rx_len<3){
            tmp_list = tmp_list->next;
            continue;
        }

		//parse
		//all_ack_data[0] = rx_len;
		//memcpy(all_ack_data+1,uds_ack_data,7);
		if(rx_len>=4){
            

			//ALOGD("LYY3 start to parse");

            //for(int i=0;i<8;i++){
               // ALOGD("LYY3 %02x",all_ack_data[i]);
           // }
			parseIVCxtAck(ack_info,uds_ack_data,rx_len);
			
			//ALOGD("LYY3 SPEED1:%f",ack_info->result);
			this_time = getCurrentTime();
			notifyCxtInfo(this_time,sid,pid,ack_info->result);
            ALOGD("LYY_YL PID:%d VALUE:%f",pid,ack_info->result);

			//update state
			switch(pid){
				case 1:
					v_run_state.speed.st = (double)ack_info->result;
					v_run_state.speed.ts = this_time;
					break;
				case 3:
					v_run_state.rpm.st = (double)ack_info->result;
					v_run_state.rpm.ts = this_time;
                    break;
				case 11:
					v_run_state.steer.st = (double)ack_info->result;
					v_run_state.steer.ts = this_time;
                    break;
			}
					
		}

		if(sys_state->is_to_exit) {
			break;
		}

//rx_len = retrieveOBDCxt(req_cid, ack_cid, obd_req_data, 2, obd_ack_data, 8);
		usleep(25*1000);
		tmp_list = tmp_list->next;
	}
    if(tmp_list==NULL){
        return true;
    }else{
        return false;
    }

}

/* =============================================== */
/* ======= Special CAN Session Function ========== */
/* =============================================== */

/* Wait the successful session construction response from the target can id 
 * for specified time.
 */
static bool waitSessionResponse(uint sk, int id) {
	int i = 0;
	struct can_frame frame;
#ifdef IS_ACTION_MODE
	while((i < 100) && (sys_state->run_mode & SYS_MODE_ACTION))
#else
	while((i < 100) && (sys_state->run_mode & SYS_MODE_CONTEXT))
#endif 
	{
		if(rxCanFrame(sk, &frame, 1000) > 0) { // receive response timout=20ms
            if(frame.can_id == id) {
				// TODO: Check the data
				// Success full ack is received
				return true;
			}
		} 
		i++;
	}
	ALOGE("No session response received from %d", id);
	return false;
}

/* Send UDS quit request frame */
static void sendLeaveSessionRequest(uint sk, uint id, struct IVSysInfo *iv_sys){
	struct ivCanFrameInfo *req = &iv_sys->leave_session_request;
	txCanFrame(sk, id, req->data, req->length);
}


/* A communication session request for entering a subsystem represted by the id */
static void sendBuildSessionRequest(uint sk, uint id, struct IVSysInfo *iv_sys) {
	struct ivCanFrameInfo *req = &iv_sys->build_session_request;
	txCanFrame(sk, id, req->data, req->length);
}

#if 0
/*
 * Quit the specified subsys by sending a FIN packet with waiting the reponse
 */
unsigned int quitSession(ushort can_id) {
	if(sys_state->can0_info->can_session_id == 0) {
		return 0;
	}
	if(sys_state->can0_info->can_session_id == can_id) {
		sendUDSQuitRequest(can_id); // Send UDS Session Request
		usleep(1000);
		sys_state->can0_info->can_session_id = 0;
		return 0;
	}
	return sys_state->can0_info->can_session_id;
}
#endif
void quitCurrentSession(struct IVSysInfo *iv_sys) {
	ALOGD("LYY5 %d",sys_state->can0_info->can_session_id);
	if(sys_state->can0_info->can_session_id > 0) {
		ALOGD("LYY5 Quitsystem");
		ALOGD("Try to quit a new session with %s", iv_sys->name);
		sendLeaveSessionRequest(sys_state->can0_info->can_sk, 
				sys_state->can0_info->can_session_id, iv_sys); // Send UDS Session request
		waitInCxt(10*1000);
		ALOGD("Leave a new session with %s", iv_sys->name);
	}
	sys_state->can0_info->can_session_id = 0;
	sys_state->can0_info->can_dst_id = 0;
	sys_state->can0_info->can_src_id = 0;
}
/*
 * Try to enter the specified subsys by constructing a session
 * service 0x2
 */
bool buildNewSession(ushort req_id, ushort ack_id, struct IVSysInfo *iv_sys) {

	if(sys_state->can0_info->can_session_id == req_id) {
		ALOGD("The CAN0 port already maintains a session with the target!"); 
		return true;
	}
    //ALOGD("LYY8 positon 1");

	if(sys_state->can0_info->can_session_id != 0) {
		quitCurrentSession(iv_sys);
	}

    //ALOGD("LYY8 positon 2");
	ALOGD("Try to build a new session with %s: -> 0x%x", iv_sys->name, req_id);

	sendBuildSessionRequest(sys_state->can0_info->can_sk, req_id, iv_sys);
	sys_state->can0_info->can_dst_id = req_id;
	sys_state->can0_info->can_src_id = ack_id;

   // ALOGD("LYY8 positon 3 : %d  name: %s",ack_id,iv_sys->name);
	// ALOGD("Try to build new session with %s: -> %d, <- %d", iv_sys->name, req_id, ack_id);

	if(!waitSessionResponse(sys_state->can0_info->can_sk, ack_id)) { // Wait for new session response
		ALOGE("Fail to build new session with %s: <- 0x%x", iv_sys->name, ack_id);
		return false; 
	}

    //ALOGD("LYY8 positon3.5");
	sys_state->can0_info->can_session_id = req_id;
    //ALOGD("LYY8 positon 4");
	ALOGD("Buit a new session with %s: -> 0x%x", iv_sys->name, req_id);
	return true;
}

/* The callback handler for processing the received CAN frame during context mode */
int rxContextFrameHandler(struct can_frame *frame, uint load_size) {
	if(!(sys_state->run_mode & SYS_MODE_CONTEXT)) {
		ALOGD("Running mode is not SYS_MODE_CONTEXT already!");
		return -1;
	}
	if(load_size > 0) { // Contain payload inform (SF, FF, or CF)
		/* Reply the frame content to the client if it runs in CONEXT reading mode */
		txClientCanFrame(0, sys_state->can0_info->rx_frame_num++, frame->can_id, frame->data, frame->can_dlc);
	}
	return 0;
}
#if 0
/* Retrieve the special in-vehicle context (service 0x22) */
static struct cxtInfo* retrieveCxtInfo(struct cxtRequest *iv_request) {
	uchar rx_buf[MAX_LOAD_SIZE];
	char  tmp[1024];
	uint  rx_len = 0;
	uint tx_len = txPayload(sys_state->can0_info->can_sk, sys_state->can0_info->can_src_id, 
			sys_state->can0_info->can_dst_id, iv_request->payload, iv_request->len); // Tx the request

	if(tx_len != iv_request->len) {
		ALOGE("Transmitting context request payload error!!!");
		return NULL;
	}

	// rx_len = rxPayload(rx_buf, MAX_LOAD_SIZE); // Rx the payload
	rx_len = rxPayload(sys_state->can0_info->can_sk, sys_state->can0_info->can_src_id, 
			rx_buf, MAX_LOAD_SIZE, &rxContextFrameHandler); // Rx the payload

	bytesToHexString(tmp, rx_buf, rx_len);
	ALOGD("%s: %d %s", __func__, rx_len, tmp);

	if(rx_len <= iv_request->len) {
		ALOGE("Received UDS frame is too small!!!");
		return NULL;
	}

	if(rx_buf[0] != 0x62) {
		ALOGD("Receied UDS frame contains invalid data!!");
		return NULL;
	}
	if(memcpy(rx_buf+1, iv_request->payload+1, iv_request->len-1) != 0) {
		ALOGD("Receied UDS frame is not from the target service!!");
		return NULL;
	}

	// TODO: Further process the payload or notice the client
	if(rx_len > 0) {
		return putNewCxtData(iv_request, rx_buf, rx_len);
	}

	return NULL;
}
#endif


/* Retrieve the context relavent to special in-vehicle system */
static void retrieveSysInfo() {

    struct IVSysInfo *t_iv_sys = used_sys_list;
    while(t_iv_sys  && (sys_state->run_mode & SYS_MODE_CONTEXT)){
        //ALOGD("LYY2222 POS1");
        struct IVReqInfo *t_iv_request = t_iv_sys->req_list;
    
        if(t_iv_request) {
            if(buildNewSession(t_iv_sys->req_id, t_iv_sys->ack_id, t_iv_sys)){
                retrieveUDSInfo(0x22, t_iv_request);
                quitCurrentSession(t_iv_sys);
            }       
        }

        t_iv_sys = t_iv_sys->next;

		if(sys_state->is_to_exit) {
			break;
		}
    }

#if 0
	if(t_iv_request) {
		// To build a new session with the specified in-vehicle system
		if(!buildNewSession(iv_sys->request_id, iv_sys->ack_id, iv_sys))
			return false;
	}

	while(t_iv_request && (sys_state->run_mode & SYS_MODE_CONTEXT)) {
		// To retrieve the tareget context of the specified system
		ALOGD("To retrieve context info related to %s", t_iv_request->name);
		//cxt_info = retrieveCxtInfo(t_iv_request);
        retrieveUDSInfo(0x22,t_iv_request);
		//if(cxt_info) {
			//txCxtPayload(((uchar*)cxt_info)+sizeof(void*), sizeof(struct cxtInfo)+cxt_info->len);
		//};
		t_iv_request = t_iv_request->next;
	}

	// Close the session
	quitCurrentSession(iv_sys);

	if(!t_iv_request) // All supportted context has be retrieved
		return true;
	return false;
#endif
}

/* Iteratively collect the in-vehicle context information */
void collectIVContext(void) {
	//struct ivSysInfo *t_iv_sys = iv_sys_list;
	// ALOGD("To collect in-vehicle context information");
	pthread_mutex_lock(&sys_state->can0_info->can_lock);

	// ALOGD("%s: Start 0x%08x %d", __func__, (uint)t_iv_sys, sys_state->run_mode);

	/* while(t_iv_sys  && (sys_state->run_mode & SYS_MODE_CONTEXT)) {
	// To collect the information of a specific in-vehicle system
	retrieveSysInfo(t_iv_sys);
	t_iv_sys = t_iv_sys->next;
	}*/

#if IS_NOT_SIMULATOR
	//retrieveOBDInfo(0x1);
	retrieveSysInfo();
    //modelTest("/sdcard/test1.log");   
#else
	//ALOGD("LYY2222 Start to retrieveSysInfo");
    //retrieveSysInfo();
	retrieveOBDInfo(0x1);
#endif

	pthread_mutex_unlock(&sys_state->can0_info->can_lock);

	return;
}


void startContextMode() {
	//if(used_sys_list)
		//return;
	//iv_sys_list = initIVContextInfo("/sdcard/skeda_context.csv");
	sys_state->run_mode	= SYS_MODE_CONTEXT;
	ALOGI("Set run_mode = 0x%x", SYS_MODE_CONTEXT);
}

