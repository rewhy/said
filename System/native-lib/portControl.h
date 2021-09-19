// portControl.h
#ifndef __CAN_CONTRL_H
#define __CAN_CONTRL_H

#include "utils.h"
#include "canitf.h"
#include "isoTp.h"



struct ivActionCtrl {
	char		name[32];
	uint		action_id;
	ushort	req_id;
	uchar		req_data[MAX_LOAD_SIZE];
	uint		req_len;
	ushort	ack_id;
	uchar		ack_data[MAX_ACK_SIZE];
	uint		ack_len;
	struct ivActionCtrl *next;
};

struct ctrlAction{
	char	name[100];
	int		decision;
	int		number;
	int		pid[100];//MAX_CONDITION_LENGTH
	int		symbol[100];
	double	value[100];     

	struct ctrlAction *next;
};

extern struct ivActionCtrl *ext_iv_act;
extern struct ctrlAction *action_rules;

/*======== Action =======*/
void ActionSuccess(int i);
void CanBoxActionLaBa();
void CanSimKaiSuo();
void CanSimShangSuo();
void CanSimKaiChuang();
void CanSimGuanChuang();
void CanSimGuanRVM();
void CanSimKaiRVM();

void doControlAction(uchar action, uchar *data);

void initControlInfo();

#endif // __CAN_CONTRL_H
