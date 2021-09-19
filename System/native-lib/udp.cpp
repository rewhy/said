/*************************************************************************
	> File Name: udp.cpp
	> Author: 
	> Mail: 
	> Created Time: 2019年11月02日 星期六 22时24分19秒
 ************************************************************************/

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
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

#include "onboard.h"
#include "ivSpec.h"

#define PORT 9999
#define BUFFLEN 1024



void handle_udp_msg(int fd){

    char buff[BUFFLEN];
    socklen_t len;
    int count;

    char *field = NULL;
    char *data = NULL;
    char type[3]={0};

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
	uchar serviceData[8] = {0};


    struct sockaddr_in client_addr;
    ALOGD("LYY111 POS1");

    v_run_state.speed.st = 0;
    v_run_state.speed.ts = 0;
    v_run_state.rpm.st = 0;
    v_run_state.rpm.ts = 0;
    v_run_state.steer.st = 0;
    v_run_state.steer.ts = 0;

    while(1){
        memset(buff,0,BUFFLEN);
        len = sizeof(client_addr);
        count = recvfrom(fd,buff,BUFFLEN,0,(struct sockaddr*)&client_addr,&len);
        if(count == -1){
            printf("receive data fail!\n");
            return;
        }
        if(count<1){
            continue;
        }
        //printf("client:%s\n",buff);
        //memset(buff,0,BUFFLEN);
        //sprintf(buff, "I have received %d bytes data!\n",count);
        //printf("server:%s\n",buff);
        //sendto(fd,buff,BUFFLEN,0,(struct sockaddr*)&client_addr,len);
        field = strtok(buff,":");
        data = strtok(NULL,":");        
        ALOGD("LYY111 %s",field);
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
				ALOGD("LYYTEST000 length:%d  %x %x %x %x %x %x %x %x",length,bytesData[0],bytesData[1],bytesData[2],bytesData[3],bytesData[4],bytesData[5],bytesData[6],bytesData[7]);
				memcpy(serviceData,bytesData+1,length-1);
                parseIVCxtAck(ack_info,serviceData,length-1);
				ALOGD("LYYTEST000 %f",ack_info->result);
                notifyCxtInfo(time,0x22,pid,ack_info->result);

                switch(pid){
                    case 1:
					    v_run_state.speed.st = (double)ack_info->result;
					    v_run_state.speed.ts = 0;
					    break;
				    case 4:
					    v_run_state.rpm.st = (double)ack_info->result;
					    v_run_state.rpm.ts = 0;
                        break;
				    case 11:
					    v_run_state.steer.st = (double)ack_info->result;
					    v_run_state.steer.ts = 0;
                        break;
                }
            }

        }else{
            continue;
        }
    }

}

int receive_UDP(){
    int server_fd , ret;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET,SOCK_DGRAM,0);
    if(server_fd<0){
        printf("Create socket fail\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
    server_addr.sin_port = htons(PORT);  //端口号，需要网络序转换

    ret = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        ALOGD("LYY111 socket bind fail!\n");
        return -1;
    }
    ALOGD("LYY111 POS0");

    handle_udp_msg(server_fd);   //处理接收到的数据

    close(server_fd);

    return 0;
}


