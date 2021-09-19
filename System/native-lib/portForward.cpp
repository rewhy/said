// portForward.cpp

#include <linux/can.h>
#include <utils/Log.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libsvm/svm.h>

#include "log.h"
#include "canitf.h"
#include "clientCom.h"
#include "isoTp.h"
#include "portForward.h"
#include "portControl.h"
#include "portContext.h"
#include "utils.h"

extern struct sysRunningStatus *sys_state;

struct startDiagnosis *obd_state;
struct startDiagnosis *tp_state;

ullong last_time;

struct canFrameFilter *filter_table = NULL;

struct formatRule kwp_rule[32];
struct formatRule uds_rule[26];
struct formatRule obd_rule[9];

struct permissionRule kwp_permission[32];
struct permissionRule uds_permission[26];
struct permissionRule obd_permission[9];

struct multipleFrame *before_frame;
struct multipleFrame *beforeUDS_frame;


struct vehicleService v_run_service;

/* On external frame filter function */
/*
 * =0: forward this frame
 * >0: filter the frame
 * <0: exit forward state
 */
int cmp2Action(uchar *act,uchar *frame,int len1,int len2){
	if(len1<4 || len2!=8 ){
		return 0;
	}
	if(len1<=7){
		for(int i=0;i<3;i++){
			if(act[i]!=frame[i+1]){
				return 0;
			}
		}
	}
	else{
		for(int i=0;i<3;i++){
			if(act[i]!=frame[i+2]){
				return 0;
			}
		}
	}		
	return 1;

}

bool checkFormula(int symbol,double x1,double x2){
	bool flag = false;
	switch(symbol){
		case 0://<
			if(x1 < x2){
				flag = true;
			}else{
				flag = false;
			}
			break;
		case 20://>
			if(x1 > x2){
				flag = true;
			}else{
				flag = false;
			}
			break; 
		case 11://==
			if(x1 == x2){
				flag = true;
			}else{
				flag = false;
			}
			break;
		case 21://>=
			if(x1 >= x2){
				flag = true;
			}else{
				flag = false;
			}
			break;
		case 1://<=
			if(x1 <= x2){
				flag = true;
			}else{
				flag = false;
			}
			break;
		case 10://!=
			if(x1 != x2){
				flag = true;
			}else{
				flag = false;
			}
			break;
	}

	return flag;
}

bool checkActionParameters(char *name,int pid,uchar *frame){
    double real_value = 0;
    
    //judge the name
    if(strcmp(name,"turn_signal_lamp")==0){
        if(pid==-1){
            real_value = (double)frame[6];
        }else if(pid == -2){
            real_value = (double)frame[7];
        }

    }
    if(strcmp(name,"frontdriverside_window")==0){
        if(pid==-1){
            real_value = (double)frame[6];
        }else if(pid == -2){
            if(frame[7]==0){
                real_value = 0;
            }else{
                real_value = 1;
            }
        }
    }
    if(strcmp(name,"reardriverside_window")==0){
        if(pid==-1){
            real_value = (double)frame[6];
        }else if(pid == -2){
            if(frame[7]==0){
                real_value = 0;
            }else{
                real_value = 1;
            }
        }
    }
    if(strcmp(name,"frontpassenger_window")==0){
        if(pid==-1){
            real_value = (double)frame[6];
        }else if(pid == -2){
            if(frame[7]==0){
                real_value = 0;
            }else{
                real_value = 1;
            }
        }
    }
    if(strcmp(name,"reardriverside_window")==0){
        if(pid==-1){
            real_value = (double)frame[6];
        }else if(pid == -2){
            if(frame[7]==0){
                real_value = 0;
            }else{
                real_value = 1;
            }
        }
    }
    return real_value;

}




//check whether the frame is setup channel in TP2.0
bool judgeTPSetup(struct can_frame *frame){
    if(frame->can_dlc==7){
        if(frame->can_id==0x200){
            if(frame->data[1]==0xC0||frame->data[1]==0xD0||frame->data[1]==0xD6||frame->data[1]==0xD7||frame->data[1]==0xD8){
                tp_state->startTime = getCurrentTime();
                last_time = tp_state->startTime;
                tp_state->status = 1;
                tp_state->can_id = frame->can_id;
                return true;
            }
        }
    } 
    return false;
}


bool calculateExpress(Expression *e, double x){
    bool res = false;
    int tmp1,tmp2;

    switch (e->symbol){
        case 0: //!=
            if(e->parameteres[0]!=x){
                res = true;
            }else{
                res = false;
            }
            break;
        case 1: //=
            if(x == e->parameteres[0]){
                res = true;
            }else{
                res = false;
            }
            break;
        case 2: //<
            if(x < e->parameteres[0]){
                res = true;    
            }else{
                res = false;
            }
            break;
        case 3: //>
            if(x > e->parameteres[0]){
                res = true;
            }else{
                res = false;
            }
            break;
        case 4: //<=
            if(x <= e->parameteres[0]){
                res = true;
            }else{
                res = false;
            }
            break;
        case 5: //>=
            if(x >= e->parameteres[0]){
                res = true;
            }else{
                res = false;
            }
            break;
        case 6: //[)
            if(x >= e->parameteres[0] && x < e->parameteres[1]){
                res = true;
            }else{
                res = false;
            }
            break;
        case 7: //[]
            if(x >= e->parameteres[0] && x <= e->parameteres[1]){
                res = true;
            }else{
                res = false;
            }
            break;
        case 9:
            if(x >= e->parameteres[0] && x <= e->parameteres[1] && ((int)x%2 != 0)){
                res = true;
            }else{
                res = false;
            }
            break;
        case 10:
            if(x >= e->parameteres[0] && x <= e->parameteres[1] && ((int)x%2 == 0)){
                res = true;
            }else{
                res = false;
            }
            break;
        case 11:
            tmp1 = (int)(x-e->parameteres[0]);
            tmp2 = tmp1 % (int)e->parameteres[1];
            if(tmp1>=0 && tmp2 == 0){
                res = true;
            }else{
                res = false;
            }
            break;
        case 12:
            for(int i=0;i<e->parameteres_len;i++){
                if(x == e->parameteres[i]){
                    res = true;
                    break;
                }
            }
            break;
    }

    return res;
}

//special it may not be TP2.0, 4 type,not data type
int checkTPType(struct can_frame *frame){
    uchar tmp;
	//Channel setup
    if(frame->can_dlc==7){
        if(frame->can_id==0x200){
            if(frame->data[1]==0xC0||frame->data[1]==0xD0||frame->data[1]==0xD6||frame->data[1]==0xD7||frame->data[1]==0xD8){
                return 1;
            } 
        }
		//Broadcast
        if(frame->data[1]==0x23||frame->data[1]==0x24){
            if((frame->data[5]==0x00||frame->data[5]==0x55||frame->data[5]==0xAA)&&(frame->data[5]==frame->data[6])){
                return 2;
            }
        }

    }
	//Channel parameters
    if(frame->can_dlc==1||frame->can_dlc==6){
        if((frame->data[0]>=0xA0&&frame->data[0]<=0xA4)||frame->data[0]==0xA8){
            return 3;
        }
    }
    
	//Data transmission    
    if(frame->can_dlc>=2){
        tmp = (frame->data[0]>>4)&0x0F;
        if(tmp<=0x03||tmp==0x0B||tmp==0x09){
            return 4;
        }
    }

    //others
    return 0;
}

int checkUDSType(struct can_frame *frame){
    uchar fhead;
    uchar fend;

    fhead =  (frame->data[0]>>4)&0x0F;
    fend =  frame->data[0]&0x0F;

    if(frame->can_dlc!=8){
        return -1;
    }

    //SF
    if(fhead == 0){
        if(fend<=7){
            return 0;
        }
    }
    //FF
    if(fhead == 1){
        return 1;
    }
    //CF
    if(fhead == 2){
        return 2;
    }
    //FC
    if(fhead == 3){
        if(fend<=2){
            return 3;
        }
    }
    //others wrong
    return -1;

}

double getStateValue(int type){
    switch(type){
        case 1:
            return v_run_state.speed.st;
            break;
        case 2:
            return v_run_state.rpm.st;
            break;
        case 11:
            return v_run_state.steer.st;
            break;
    }
    return 0;
}

int checkPermission(struct permissionRule* permission,int index,uchar *buff,int len_max){
    struct singlePermission *tmp = NULL;
    struct condition *ctmp = NULL;
    struct vehicleState *vtmp = NULL;

    bool flag = true;
    int tmp_byte = 0;
    double state_value=0;

    tmp = permission[index].head_permission;
    //printf("TEST serviceID:%x",permission[index].serviceID);
    while(tmp){
        flag = true;
        ctmp = tmp->head_rule;
        while(ctmp){
            if(ctmp->byte > len_max){
                break;
            }
            tmp_byte = ctmp->byte - 1;  //#1 == byte 0 --> buff[0]
            if(!calculateExpress(&ctmp->exp,buff[tmp_byte])){
                flag = false;
                break;
            }
            ctmp = ctmp->next;
        }
        if(flag == true){
            vtmp = tmp->head_state;
            while(vtmp){
                state_value = getStateValue(vtmp->state_type);
                if(!calculateExpress(&vtmp->exp,state_value)){
                    flag = false;
                    break;
                }

                vtmp = vtmp->next;
            }
            if(flag == true){
                return tmp->decision;
            }
        }

        tmp = tmp->next;
    }

    return 1;
}

bool judgeUDSSetup(struct can_frame *frame){
    uchar buff[8] = {0};
    int result;

    if(frame->can_dlc==8){
        if(frame->can_id>=0x700){
            if(frame->data[1]==0x10){
                memcpy(buff,frame->data+1,7);
                result = checkPermission(uds_permission,0,buff,8);
                
                if(result > 0){
                    memset((void*)obd_state,0,sizeof(struct startDiagnosis));
                    obd_state->startTime = getCurrentTime();
                    last_time = obd_state->startTime;
                    obd_state->status = 1;
                    obd_state->can_id = frame->can_id;
                    obd_state->session_type = buff[1];
                    return true;
                }
            }
        }
    }
    return false;
}


bool checkDependency(uchar sid){
    if(sid == 0x36){
        if(v_run_service.serviceID == 0x34 || v_run_service.serviceID == 0x35 || v_run_service.serviceID == 0x36){
            return true;
        }else{
            return false;
        }
    }

    if(sid == 0x37 || sid == 0x38){
        if(v_run_service.serviceID == 0x36){
            return true;
        }else{
            return false;
        }
    }

    return true;
}

bool checkSession(uchar sid){
    //obd_state->session_type
    if(obd_state->session_type==0x01 || obd_state->session_type==0x00){
        return false;
    }

    if(obd_state->session_type==0x04){
        return false;
    }
    
    return true;
}


int judgeTPData(uchar *buff,int len,int data_type){
    int i=0;
    int j=0;
    int index = -1;
    bool flag;
    int tmp_byte=0;
	int length = 0;
	int result = 0;
    //struct formatRule *tmp_format = kwp_format;
    struct singleRule *tmp = NULL;
    struct condition *ctmp = NULL;

    for(i=0;i<32;i++){
        if(kwp_rule[i].serviceID==buff[0]){
            index = i;
            break;
        }
    }

    if(index<0){
        ALOGD("There is no KWP serviceID!");
        return 0;
    }

    if(!checkDependency(buff[0])){
        return 0;
    }
    //SF
    if(data_type==0){
        tmp = kwp_rule[index].hrule;
        while(tmp){
            if(calculateExpress(&tmp->length,len)){
                flag = true;
                ctmp = tmp->con;
                while(ctmp){
                    tmp_byte = ctmp->byte - 1;
                    if(!calculateExpress(&ctmp->exp,buff[tmp_byte])){
                        flag = false;
                        break;
                    }

                    ctmp = ctmp->next;
                }
                if(flag == true){
					result =  checkPermission(kwp_permission,index,buff,len);
                    if(result > 0){
                        v_run_service.serviceID = buff[0];
                    }
                    return result;
                    //return 1;
                }
            }

            tmp = tmp->next;
        }
    }

    //CF
    if(data_type==1){
        tmp = kwp_rule[index].hrule;
        while(tmp){
            if(calculateExpress(&tmp->length,len)){           
                flag = true;
                ctmp = tmp->con;
                while(ctmp){
                    tmp_byte = ctmp->byte - 1;
                    if(tmp_byte > 4){
                        break;
                    }
                    if(!calculateExpress(&ctmp->exp,buff[tmp_byte])){
                        flag = false;
                        break;
                    }

                    ctmp = ctmp->next;
                }
                if(flag==true){
                    //return checkPermission(kwp_permission,index,buff,5);
                    result = checkPermission(kwp_permission,index,buff,5);
                    if(result > 0){
                        v_run_service.serviceID = buff[0];
                    }
                    return result;
                    //return 1;
                }
            }
			tmp = tmp->next;
        }
    }

#if 0 
    while(tmp_format){
        
        if(tmp_format->serviceID==buff[0]){
            if(data_type==0||data_type==3){
                
                for(i=0;i<tmp_format->types;i++){
                    //check Singlerule
                    if(calculateExpress(tmp_format->type[i].length,len)){
                        flag = true;
                        for(j=0;j<tmp_format->type[i].conditionLength;j++){
                            //single condition
                            tmp_byte = tmp_format->type[i].con[j].byte - 1;
                            if(!calculateExpress(tmp_format->type[i].con[j].e,buff[tmp_byte])){
                                flag = false;
                                break;
                            }
                        }
                        if(flag==true){
                            return 1;
                        }
                    }
                }
            }


            if(data_type==1||data_type==2){
                for(i=0;i<tmp_format->types;i++){
                    flag = true;
                    for(j=0;j<tmp_format->type[i].conditionLength;j++){
                        tmp_byte = tmp_format->type[i].con[j].byte - 1; // #1#2#2~~~bit 012
                        if(tmp_byte > len - 1){
                            break;          //continue;
                        }
                        if(!calculateExpress(tmp_format->type[i].con[j].e,buff[tmp_byte])){
                            flag = false;
                            break;
                        }
                        if(flag == true){
                            return 1;
                        }

                    }
                }
            }

            

            break;
        }

        tmp_format = tmp_format->next;
    }
#endif

    return 0;
    
}    

//TODO
int judgeUDSData(uchar *buff,int len,int data_type){
    int i=0;
    int j=0;
    int index = -1;
    bool flag;
    int tmp_byte=0;
	int result = 0;
    
    struct singleRule *tmp = NULL;
    struct condition *ctmp = NULL;

    for(i=0;i<26;i++){
        if(uds_rule[i].serviceID==buff[0]){
            index = i;
            break;
        }
    }
	
	//ALOGD("LYY428 Enter uds_permission");
	//TOCHANGE
	//return checkPermission(uds_permission,index,buff,len);

    if(!checkDependency(buff[0])){
		ALOGD("LYY428 dependency wrong");
        return 0;
    }
    if(!checkSession(buff[0])){
        ALOGD("LYY428 session wrong\n");
        return 0;
    }
	//ALOGD("LYY428 ENTER Second Part data_type:%d",data_type);

    if(index<0){
        ALOGD("There is no UDS serviceID!");
        return 0;
    }

    if(data_type==0){
        tmp = uds_rule[index].hrule;
        while(tmp){
            if(calculateExpress(&tmp->length,len)){
                flag = true;
                ctmp = tmp->con;
                while(ctmp){
                    tmp_byte = ctmp->byte - 1;
                    if(!calculateExpress(&ctmp->exp,buff[tmp_byte])){
                        flag = false;
                        break;
                    }
                    ctmp = ctmp->next;
                }
                if(flag == true){
                    //return checkPermission(uds_permission,index,buff,len);
					//TOCHANGE
                    //return 1;   
					result = checkPermission(uds_permission,index,buff,len);
                    if(result>0){
                        v_run_service.serviceID = buff[0];
                        if(obd_state->error_code>=10){
                            printf("Error %d\n",obd_state->error_code);
                            obd_state->status = 0;
                        }
                    }
                    return result;
                }
            }

            
            tmp = tmp->next;
        }
    }

    if(data_type==1){
        tmp = uds_rule[index].hrule;
        while(tmp){
            if(calculateExpress(&tmp->length,len)){
                flag = true;
                ctmp = tmp->con;
                while(ctmp){
                    tmp_byte = ctmp->byte - 1;
                    if(tmp_byte > 5){
                        break;
                    }
                    if(!calculateExpress(&ctmp->exp,buff[tmp_byte])){
                        flag = false;
                        break;
                    }

                    ctmp = ctmp->next;
                }
                if(flag==true){
                    //return checkPermission(uds_permission,index,buff,6);
					//TOCHANGE
                    //return 1;
					result = checkPermission(uds_permission,index,buff,6);
                    if(result>0){
                        v_run_service.serviceID = buff[0];
                    }
                    return result;
                }
            }
			tmp = tmp->next;
        }
    }
#if 0
    while(tmp_format){
        
        if(tmp_format->serviceID==buff[0]){
            if(data_type==0){
                for(i=0;i<tmp_format->types;i++){
                    //check singlerule
                    if(calculateExpress(tmp_format->type[i].length,len)){
                        flag = true;
                        for(j=0;j<tmp_format->type[i].conditionLength;j++){
                            //single condition
                            tmp_byte = tmp_format->type[i].con[j].byte - 1;
                            if(!calculateExpress(tmp_format->type[i].con[j].e,buff[tmp_byte])){
                                flag = false;
                                break;
                            }
                        }
                        if(flag==true){
                            return 1;
                        }
                    }
                }

            }

            if(data_type==1||data_type==2){
                for(i=0;i<tmp_format->types;i++){
                    flag = true;
                    for(j=0;j<tmp_format->type[i].conditionLength;j++){
                        tmp_byte = tmp_format->type[i].con[j].byte - 1; // #1#2#2~~~bit 012
                        if(tmp_byte > len - 1){
                            break;          //continue;
                        }
                        if(!calculateExpress(tmp_format->type[i].con[j].e,buff[tmp_byte])){
                            flag = false;
                            break;
                        }
                        if(flag == true){
                            return 1;
                        }

                    }
                }   
            }

            break;
        }

        tmp_format = tmp_format->next;
    }
#endif

    return 0;
}

int judgeOBDData(uchar *buff,int len){

    int index = -1;
    bool flag;
    int tmp_byte=0;
    
    struct singleRule *tmp = NULL;
    struct condition *ctmp = NULL;

    for(int i=0;i<9;i++){
        if(obd_rule[i].serviceID==buff[0]){
            index = i;
            break;
        }
    }

    if(index<0){
        printf("There is no UDS serviceID!");
        return 0;
    }

    tmp = obd_rule[index].hrule;
    while(tmp){
        if(calculateExpress(&tmp->length,len)){
            flag = true;
            ctmp = tmp->con;
            while(ctmp){
                tmp_byte = ctmp->byte - 1;
                if(!calculateExpress(&ctmp->exp,buff[tmp_byte])){
                    flag = false;
                    break;
                }
                ctmp = ctmp->next;
            }
            if(flag == true){
                return checkPermission(obd_permission,index,buff,len);
                //return 1;   
            }
        }

            
        tmp = tmp->next;
    }

    return 0;
}

//resolve data frame
int checkTPDataFrame(struct can_frame *frame,ullong ts){
    uchar tmp;
    uchar seq;
    int result=0;
    uchar buff[8]={0};
    int length = 0;

    tmp = (frame->data[0]>>4)&0x0F;
    seq = frame->data[0]&0x0F;

    if(tmp==0x0B||tmp==0x09){
        return 1;
    }

    
    if(tmp==0x01||tmp==0x03){
        //Single frame 0
        if(before_frame->multiple_num==0){
			if(frame->can_dlc<3){
                return 0;
            }
            length = length | frame->data[2] | (frame->data[1]<<8);
            if(length != frame->can_dlc-3){
                return 0;
            }
            memset((void*)before_frame,0,sizeof(struct multipleFrame));
            memcpy(buff,frame->data+3, frame->can_dlc - 3);
            return judgeTPData(buff, length ,0);
        }//Last multiple frame 3
        else{
			if(frame->can_dlc<1){
                return 0;
            }
            memcpy(before_frame->bf_data,frame->data+1,frame->can_dlc-1);
            before_frame->bfdata_len = before_frame->bfdata_len + frame->can_dlc-1;
            //result = judgeTPData(before_frame->bf_data,before_frame->bfdata_len,3);
            result = before_frame->bf_res;

            memset((void*)before_frame,0,sizeof(struct multipleFrame));
            return result;
        }
        
    }
    
    if(tmp==0x00||tmp==0x02){
        //FF 1
        if(before_frame->multiple_num==0){
            length = length | frame->data[2] | (frame->data[1]<<8);
			if(frame->can_dlc!=8 && length > 5){
                return 0;
            }
            //memcpy(buff,frame->data+3,frame->can_dlc-3);
            memset((void*)before_frame,0,sizeof(struct multipleFrame));

            before_frame->bf_option=tmp;
            before_frame->bf_seq = seq;
            before_frame->multiple_num = before_frame->multiple_num + 1;
            memcpy(before_frame->bf_data + before_frame->bfdata_len,frame->data+3, frame->can_dlc - 3);
            before_frame->bfdata_len = before_frame->bfdata_len +  frame->can_dlc - 3;
            before_frame->bf_time = ts;           
            
            before_frame->bf_res = judgeTPData(before_frame->bf_data,length,1);

            return before_frame->bf_res;
        }else if(seq == ((before_frame->bf_seq+1)%16)){
			if(frame->can_dlc<1){
                return 0;
            }
            if(before_frame->bfdata_len > 1000){
                memset((void*)before_frame->bf_data,0,1024); 
                before_frame->bfdata_len = 0;
            }
            before_frame->bf_option=tmp;
            before_frame->bf_seq = seq;
            before_frame->multiple_num = before_frame->multiple_num + 1;
            memcpy(before_frame->bf_data + before_frame->bfdata_len,frame->data+1,frame->can_dlc-1);
            before_frame->bfdata_len = before_frame->bfdata_len + frame->can_dlc-1;
            before_frame->bf_time = ts;           
            
            //before_frame->bf_res = judgeTPData(before_frame->bf_data,before_frame->bfdata_len,2);
            
            return before_frame->bf_res;
            
        }
        
    }

	return 0;
    
}

int checkUDSDataFrame(struct can_frame *frame,ullong ts){
    uchar fhead;
    uchar fend;
    int type;
    int result = 0;
    uchar buff[8] = {0};
    int length = 0;

    type = checkUDSType(frame);
    fhead =  (frame->data[0]>>4)&0x0F;
    fend =  frame->data[0]&0x0F;

    if(frame->can_dlc!=8){
        return 0;
    }

    if(type == 3){
        return 1;
    }

    if(type < 0){
        return 0;
    }

    if(type == 0){
        length = frame->data[0];
        memcpy(buff,frame->data+1,length);

		//ALOGD("LYY428 Enter judgeUDSData");
        return judgeUDSData(buff,length,0);
    }
    //FF
    if(type == 1){
        memset((void*)beforeUDS_frame,0,sizeof(struct multipleFrame));

        length = length | frame->data[1] | (fend<<8);
        beforeUDS_frame->bf_seq = 0;
        beforeUDS_frame->multiple_num = 1;
        memcpy(beforeUDS_frame->bf_data,frame->data+2,frame->can_dlc-2);
        beforeUDS_frame->bfdata_len = frame->can_dlc-2;
        beforeUDS_frame->bf_time = ts;

        beforeUDS_frame->bf_res = judgeUDSData(beforeUDS_frame->bf_data,length,1);
        
        return beforeUDS_frame->bf_res;
    }

    if(type == 2){
        if(fend != (beforeUDS_frame->bf_seq + 1)%16 ){
            return 0;
        }
        if(beforeUDS_frame->bfdata_len > 1000){
            beforeUDS_frame->bfdata_len = 0;
            memset((void*)beforeUDS_frame->bf_data,0,1024);
        }   
        
        beforeUDS_frame->bf_seq = fend;
        beforeUDS_frame->multiple_num = beforeUDS_frame->multiple_num + 1;
        memcpy(beforeUDS_frame->bf_data + beforeUDS_frame->bfdata_len,frame->data+1,frame->can_dlc-1);
        beforeUDS_frame->bfdata_len = beforeUDS_frame->bfdata_len + frame->can_dlc - 1;
        beforeUDS_frame->bf_time = ts;

        //beforeUDS_frame->bf_res = judgeUDSData(beforeUDS_frame->bf_data,beforeUDS_frame->bfdata_len,2);
        return beforeUDS_frame->bf_res;

    }

	return 0;

}

int checkOBDDataFrame(struct can_frame *frame){
    int type;
    int length = 0;
    uchar buff[8] = {0};

    type = checkUDSType(frame);

    if(type != 0){
        return 0;
    }
    length = frame->data[0];
    memcpy(buff,frame->data+1,length);
    return judgeOBDData(buff,length);

}

//
int checkFormatRule(struct can_frame *frame){
    ullong ts;
    int type;
    int res = 0;
    int tmp;

    ts = getCurrentTime();

    //TODO less than 0.5ms will be refused(changed to the time you want)
    if(ts-last_time<500){
        return 0;
    }

    //Diagnositic messages
    if(frame->can_id<0x700&&frame->can_id!=0x200){
        return 0;
    }

    res = checkOBDDataFrame(frame);
    if(res >= 1){
        //printf("OBD Rule is right!\n");
        last_time = ts;
        return res;    
    }

    if(judgeTPSetup(frame)){
        return 1;
    }
    if(judgeUDSSetup(frame)){
		v_run_service.serviceID = 0x10;
        return 1;
    }

    if(tp_state->status == 1){
        if(ts-tp_state->startTime<=2000000){
            //resolve or check the result
            type = checkTPType(frame);
            if(type>=1&&type<=3){
                tp_state->startTime = ts;
                last_time = ts;
                return 1;
            }
            if(type==4){
                res = checkTPDataFrame(frame,ts);
                if(res >= 1){
                    tp_state->startTime = ts;
                    last_time = ts;
                    return res;
                }
            }
            
        }
        else{
            tp_state->status = 0;
            memset((void*)before_frame,0,sizeof(struct multipleFrame));
        }
        
    }
    

    if(obd_state->status == 1){
		//ALOGD("LYY428 UDS");
        if(ts-obd_state->startTime<=2000000){
            if(frame->can_dlc==8&&frame->can_id==obd_state->can_id){
                res =checkUDSDataFrame(frame,ts);
				//ALOGD("LYY428 %d",res);
                if(res >= 1){
                    obd_state->startTime = ts;
                    last_time = ts;
                    return res;
                }
            }
            

        }else{
            obd_state->status = 0;//connect over
            obd_state->startTime = 0;
	        obd_state->session_type = 0;
            obd_state->error_code = 0;		
			memset((void*)obd_state,0,sizeof(struct startDiagnosis));
            memset((void*)beforeUDS_frame,0,sizeof(struct multipleFrame));
        }
        
    }
#if 0
    res = checkOBDDataFrame(frame);
    if(res == 1){
        //ALOGD("OBD Rule is right!\n");
        last_time = ts;
        return res;    
    }
#endif

    return 0;
    
}


int externalFrameDetect(struct can_frame *frame) {
	//txClientCanFrame(1, sys_state->can1_info->rx_frame_num++, frame->can_id, frame->data, frame->can_dlc);
    struct ivActionCtrl *iv_act = ext_iv_act;
	//ALOGD("LYY427 Start2222");
//TOCHANGE
	
    if(checkFormatRule(frame)==0){
		ALOGD("LYY427 Not pass the filter %x",frame->can_id);
        return 1;
    }

	//ALOGD("LYY427 Pass the filter");
    while(iv_act){
#if IS_NOT_SIMULATOR
		if(cmp2Action(iv_act->req_data,frame->data,iv_act->req_len,frame->can_dlc)==1){
			break;
		}
		iv_act = iv_act->next;
#else
        if(cmp2uchar(iv_act->reqet run_mode = 0x2_data,frame->data,iv_act->req_len,frame->can_dlc)==1)
            break;
        iv_act = iv_act->next;
#endif
    }

    if(iv_act == NULL){
        ALOGD("LYYA NO ACT");
        return 0;//Can't analysis this actions
        
    }
    //rules
    ALOGD("LYYA ACT_NAME: %s, speed=%f, steer=%f",iv_act->name, v_run_state.speed.st, v_run_state.steer.st);
	
	struct ctrlAction *ctr_rule = action_rules;

	int num;
	double state_value=0;
	bool flag = true;
	uchar buff[10] = {0};

	while(ctr_rule){
		//ALOGD("LYYA before ENTER %s",ctr_rule->name);
		if(strcmp(iv_act->name,(char *)ctr_rule->name)==0){
			//ALOGD("LYYA ACT_NAME: ENTER %s",iv_act->name);

			flag = true;
			if(ctr_rule->number<1){
				continue;
			}
			for(num = 0;(num < ctr_rule->number) && flag ;num++){
                //check the parameteres
                if(ctr_rule->pid[num]<0){
                    state_value = checkActionParameters(iv_act->name,ctr_rule->pid[num],frame->data);
                    ALOGD("LYYA PID STATE_VALUE = %f\n",state_value);
                }
                else{
				//check PID
				    switch(ctr_rule->pid[num]){
					    case 1:
						    state_value = v_run_state.speed.st;
						    break;
					    case 2:
						    break;
					    case 11:
						    state_value = v_run_state.steer.st;
						    break;
				    }
                }
				//choose symbols
				flag = checkFormula(ctr_rule->symbol[num],state_value,ctr_rule->value[num]);

                if(flag==false){
                    break;
                }

			}
			if(flag==true){
				ALOGD("LYYRES: %d",ctr_rule->decision);
				buff[0] = iv_act->action_id;
				buff[1] = ctr_rule->decision;
				txClientDecision(buff,2);
				return ctr_rule->decision;
			}
		}

		ctr_rule = ctr_rule->next;
	}

#if 0    
#endif
//ALOGD("LYY I JUST WANT TO SEE");
	return 0;
}

/* Init the external frame filter table (like the netfilter table) */
void initFilterHookTable() {
	struct canFrameFilter *tmp = NULL, *new_node = NULL;
	new_node = (struct canFrameFilter *)malloc(sizeof(struct canFrameFilter));
	if( new_node == NULL ) {
		ALOGE("Malloc filter node error!!!");
		return;
	}
	
	ALOGD("%s: Initialize hook table for detecting entering frames!", __func__);
	
	memset((void*)new_node, 0, sizeof(struct canFrameFilter));
	new_node->handler = &externalFrameDetect;
	
	if( filter_table == NULL ) {
		filter_table = new_node;
	} else {
		tmp = filter_table;
		while(tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new_node;
	}
}

void freeFilterHookTable() {
	struct canFrameFilter *tmp = NULL, *next = NULL;
	tmp = filter_table;
	while(tmp) {
		next = tmp->next;
		free(tmp);
		tmp = next;
	}
}

/**
 * Detect the frame coming from the internal and external ports:
 *	1) =0: Just forward the frame
 *	2) >0: Filter the frame
 *	3) <0: To exit this forward state
 */

int internalFrameHandler(struct can_frame *frame) {
	ALOGD("LYY333 POS1");
	return 0;
}


/*
 * > 0: forward the frame
 * = 0: filter the frame
 * < 0: exit the forward state
 */
int externalFrameHandler(struct can_frame *frame) {
	struct canFrameFilter *tmp = filter_table;
	int res;
	ALOGD("LYY427 %s: Frame received from the external port!", __func__);
	if(sys_state->run_mode != SYS_MODE_FORWARD) {
		sys_state->run_mode = SYS_MODE_FORWARD; // Change to FORWARD mode
		usleep(1000);
		ALOGD("LYY427 Set run_mode = 0x%x", SYS_MODE_FORWARD);
	}
    //ALOGD("LYY427 POSTION222");
	while(tmp) {
		res = (*tmp->handler)(frame);
		if(res <= 0) {
			return res;
		}
		tmp = tmp->next;
	}
	return 1;
}

int internalPortForward(void) {
	int res = 0;
	ALOGD("Entering Forward mode.");
	while(sys_state->run_mode == SYS_MODE_FORWARD) {
		ALOGD("LYY333 out start");
		// Forward all internal frame to the external port
		// res = forwardFrame(sys_state->can0_info->can_sk, sys_state->can1_info->can_sk, internalFrameHandler);
		res = forwardFrame(sys_state->can0_info->can_sk, sys_state->can1_info->can_sk, internalFrameHandler);
#if 0	
		if(sys_state->run_mode == SYS_MODE_FORWARD) {
			sys_state->run_mode = SYS_MODE_CONTEXT; // Reset the CONTEXT mode
			ALOGD("Set run_mode = 0x%x", SYS_MODE_CONTEXT);
		}
#endif
	}

	return 0;
}

/* Monitor the external port (CAN1) for forwarding the input frames */
int externalPortForward(void) {
	int res = 0;
	//ALOGD("LYY427 start1");
	if(sys_state->can1_info == NULL || sys_state->can0_info == NULL) {
		sleep(1);
		return 0;
	}
    //ALOGD("LYY427 POSTION0");
	res = forwardFrame(sys_state->can1_info->can_sk, sys_state->can0_info->can_sk, externalFrameHandler);
	if(sys_state->run_mode == SYS_MODE_FORWARD) {
	//	usleep(800*1000);
        usleep(200*1000);
		sys_state->run_mode = SYS_MODE_CONTEXT; // Reset the CONTEXT mode
		ALOGD("Set run_mode = 0x%x", SYS_MODE_CONTEXT);
	}
	
	return 0;
}

struct splitString *split(char *buff,char const *delimiter){
    struct splitString *ss;
    ss = (struct splitString *)malloc(sizeof(struct splitString)) ;
    memset((void*)ss,0,sizeof(struct splitString));    

    ss->res = strtok(buff,delimiter);
    ss->remain = strtok(NULL,"");

    return ss;
}

Expression getExpress(char *exp){
    Expression e;
    char *field = NULL;
    int num = 0;
    //printf("getE__  %s\n",exp);
    
    field = strtok(exp,"(),");
    e.symbol = atoi(field);
    field = strtok(NULL,"(),");
    e.parameteres_len = atoi(field);
    for(int i=0;i<e.parameteres_len;i++){
        field = strtok(NULL,"(),");
        e.parameteres[num] = atof(field);
        num = num + 1;
        //printf("%f\n",e.parameteres[num-1]);
    }

    return e;

}

void getFormatCondition(struct singleRule *sr,char *e){
    struct condition *ctmp = NULL, *cnew_node = NULL;   

    cnew_node = (struct condition *)malloc(sizeof(struct condition));
    if(cnew_node==NULL){
        return;
    }
    memset((void *)cnew_node,0,sizeof(struct condition));

    char *field = NULL;
    SpString *ss;
    //field = strtok(e,"(),");
    ss = split(e,"(),");
    field = ss->res;
    cnew_node->byte = atoi(field);
    //printf("%d",cnew_node->byte);
    //field = strtok(NULL,"");
    cnew_node->exp = getExpress(ss->remain);

    if(sr->con == NULL){
        sr->con = cnew_node;
    }else{
        ctmp = sr->con;
        while(ctmp->next){
            ctmp = ctmp->next;
        }
        ctmp->next = cnew_node;
    }
    //sr->conditionLength = sr->conditionLength + 1;
    
}

void addPermissionCondition(struct singlePermission *sp,char *e){
    struct condition *ctmp = NULL, *cnew_node = NULL;   

    cnew_node = (struct condition *)malloc(sizeof(struct condition));
    if(cnew_node==NULL){
        return;
    }
    memset((void *)cnew_node,0,sizeof(struct condition));

    char *field = NULL;
    SpString *ss;
    ss = split(e,"(),");
    field = ss->res;
    cnew_node->byte = atoi(field);
    //printf("condition byte:%d\n",cnew_node->byte);
    cnew_node->exp = getExpress(ss->remain);

    if(sp->head_rule == NULL){
        sp->head_rule = cnew_node;
    }else{
        ctmp = sp->head_rule;
        while(ctmp->next){
            ctmp = ctmp->next;
        }
        ctmp->next = cnew_node;
    } 
}

void addPermissionState(struct singlePermission *sp,char *e){
    struct vehicleState *vtmp = NULL, *vnew_node = NULL;

    vnew_node = (struct vehicleState *)malloc(sizeof(struct vehicleState));
    if(vnew_node==NULL){
        return;
    }
    memset((void *)vnew_node,0,sizeof(struct vehicleState));

    char *field = NULL;
    SpString *ss;
    ss = split(e,"(),");
    field = ss->res;
    vnew_node->state_type = atoi(field);
    //printf("State-Type:%d\n",vnew_node->state_type);
    vnew_node->exp = getExpress(ss->remain);

    if(sp->head_state == NULL){
        sp->head_state = vnew_node;
    }else{
        vtmp = sp->head_state;
        while(vtmp->next){
            vtmp = vtmp->next;
        }
        vtmp->next = vnew_node;
    }
}
#if 0
void initSID(struct formatRule *rule,int len,uchar *sid){
    int i;

    for(i=0;i<len;i++){
        rule[i].serviceID = sid[i];
    }
}
#endif

void initSID(struct formatRule *rule,struct permissionRule *permission,int len,unsigned char *sid){
    memset(rule,0,sizeof(struct formatRule)*len);
    memset(permission,0,sizeof(struct permissionRule)*len);   
    
    int i;
    
    for(i=0;i<len;i++){
        rule[i].serviceID = sid[i];
        permission[i].serviceID = sid[i];
    }

}

void initSingleRule(struct formatRule *rule,int len, char *buff){
    int index = -1;
    int num;

    char *field = NULL;
    char *remainning = NULL;
    SpString *sp;  

    struct singleRule *tmp = NULL, *new_node = NULL;
    struct condition *ctmp = NULL, *cnew_node = NULL;

    new_node = (struct singleRule *)malloc(sizeof(struct singleRule));
    if(new_node == NULL){
        return;
    }
    memset((void *)new_node,0,sizeof(struct singleRule));

   cnew_node = (struct condition *)malloc(sizeof(struct condition));
    if(cnew_node==NULL){
        return;
    }
    memset((void *)cnew_node,0,sizeof(struct condition));

    sp = split(buff," ");
    field = sp->res;
    remainning = sp->remain;
    unsigned int x = strtol(field,NULL,16);
    //printf("%s %s",buff,field);


    for(int i=0;i<len;i++){
        if(x==rule[i].serviceID){
            index=i;
            break;
        }
    }

    if(index == -1){
        ALOGD("No ID");
        return;
    }
    //printf("11111 %s\n",remainning);

    sp = split(remainning," ");
    field = sp->res;
    remainning = sp->remain;
    //printf("%s\n",sp->res);

    new_node->length = getExpress(field);

    sp = split(remainning," ");
    field = sp->res;
    remainning = sp->remain;
    num = atoi(field);
    new_node->conditionLength = num;
    //printf("%d\n",new_node->conditionLength);   

    for(int j=0;j<num;j++){
        sp = split(remainning," ");
        field = sp->res;
        remainning = sp->remain;       
        getFormatCondition(new_node,field);
    }

    if(rule[index].types == 0){
        rule[index].hrule = new_node;
    }else{
        tmp = rule[index].hrule;
        while(tmp->next){
            tmp = tmp -> next;
        }
        tmp->next = new_node;
    }


    rule[index].types = rule[index].types + 1;    
}

void initSinglePermission(struct permissionRule* permission,int len,char *buff){
    int index = -1,dec = 0;
    int conLen = 0,stateLen = 0;
    int num;

    char *field = NULL;
    char *remainning = NULL;
    SpString *sp;    

    struct singlePermission *tmp = NULL, *new_node = NULL;
    struct condition *ctmp = NULL, *cnew_node = NULL;
    struct vehicleState *vtmp = NULL, *vnew_node = NULL;

    new_node = (struct singlePermission *)malloc(sizeof(struct singlePermission));
    if(new_node == NULL){
        return;
    }
    memset((void *)new_node,0,sizeof(struct singlePermission));


    sp = split(buff," ");
    field = sp->res;
    remainning = sp->remain;
    unsigned int x = strtol(field,NULL,16);

    for(int i=0;i<len;i++){
        if(x==permission[i].serviceID){
            index=i;
            break;
        }
    }

    if(index == -1){
        ALOGD("No ID\n");
        return;
    }

    sp = split(remainning," conditions:");
    field = sp->res;
    remainning = sp->remain;
    conLen = atoi(field);

    for(int j=0;j<conLen;j++){
        if(remainning==NULL){
            break;
        }
        sp = split(remainning," ");
        field = sp->res;
        remainning = sp->remain;
        addPermissionCondition(new_node,field);
    }

    sp = split(remainning," states:");
    field = sp->res;
    remainning = sp->remain;
    stateLen = atoi(field);

    for(int j=0;j<stateLen;j++){
        if(remainning==NULL){
            break;
        }
        sp = split(remainning," ");
        field = sp->res;
        remainning = sp->remain;
        addPermissionState(new_node,field);
    }

    sp = split(remainning," decision:");
    field = sp->res;
    remainning = sp->remain;
    dec = atoi(field);
    new_node->decision = dec;

    if(permission[index].head_permission == NULL){
        permission[index].head_permission = new_node;
    }else{
        tmp = permission[index].head_permission;
        while(tmp->next){
            tmp = tmp->next;
        }
        tmp->next = new_node;
    }

    permission[index].number = permission[index].number + 1;

    //printf("LAST:%s===%s\n",field,remainning);  
    //printf("INDEX:%d\n",index);
    //printf("PERMISSION NUM:%d\n",permission[index].number);
}


void loadRules(struct formatRule *rule,int len, char const *name){
    char buff[255];

    FILE *fp = fopen(name, "r");
    if(!fp){
        ALOGE("RULES: Can't open file %s", name);
        return;
    }

    while(fgets(buff,255,fp)){
        ALOGD("RULES READ: %s", buff);

        initSingleRule(rule,len,buff);
    }
    fclose(fp);
}

void loadPermission(struct permissionRule *permission,int len, char const *name){
    
    char buff[255];
    
    FILE *fp = fopen(name,"r");
    if(!fp){
        ALOGE("RULES: Can't open file %s", name);
        return;
    }

    while(fgets(buff,255,fp)){
        initSinglePermission(permission,len,buff);
    }
    fclose(fp);
    
}

void initFormatRule(){
    //initFormat("/sdcard/kwp");
    //
    last_time = 0;
	v_run_service.serviceID = 0;

	obd_state = (struct startDiagnosis*)malloc(sizeof(struct startDiagnosis));
	tp_state = (struct startDiagnosis*)malloc(sizeof(struct startDiagnosis));

	memset((void*)obd_state,0,sizeof(struct startDiagnosis));
	memset((void*)tp_state,0,sizeof(struct startDiagnosis));

    before_frame = (struct multipleFrame*)malloc(sizeof(struct multipleFrame));
    memset((void*)before_frame,0,sizeof(struct multipleFrame));
    
    beforeUDS_frame =  (struct multipleFrame*)malloc(sizeof(struct multipleFrame));
    memset((void*)beforeUDS_frame,0,sizeof(struct multipleFrame));

    uchar kwpSID[32]={0x10,0x11,0x12,0x13,0x14,0x17,0x18,0x1A,0x20,0x21,0x22,0x23,0x26,0x27,0x2C,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3D,0x3E,0x80};
    uchar udsSID[26]={0x10,0x11,0x27,0x28,0x3E,0x83,0x84,0x85,0x86,0x87,0x22,0x23,0x24,0x2A,0x2C,0x2E,0x3D,0x14,0x19,0x2F,0x31,0x34,0x35,0x36,0x37,0x38};
    uchar obdSID[9]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};

    //initSID(kwp_rule,32,kwpSID);
    //initSID(uds_rule,26,udsSID);
    //initSID(obd_rule,9,obdSID);
    initSID(kwp_rule,kwp_permission,32,kwpSID);
    initSID(uds_rule,uds_permission,26,udsSID);
    initSID(obd_rule,obd_permission,9,obdSID);

    loadRules(kwp_rule,32,"/sdcard/kwp_format");
    loadRules(uds_rule,26,"/sdcard/uds_format");
    loadRules(obd_rule,9,"/sdcard/obd_format");

    loadPermission(uds_permission,26,"/sdcard/uds_permission");
    loadPermission(kwp_permission,32,"/sdcard/kwp_permission");
    loadPermission(obd_permission,9,"/sdcard/obd_permission");

    
}
