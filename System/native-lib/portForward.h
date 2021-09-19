// portForward.h
#ifndef __PORT_FORWARD_H
#define __PORT_FORWARD_H

#include <linux/can.h>

struct canFrameFilter {	
	int (*handler)(struct can_frame *);
	struct canFrameFilter *next;
};
#if 0
struct multipleFrame{
	uchar bf_option;
	uchar bf_seq;
	int bf_res; //if before frames have wrong, after ignore
	int multiple_num;
	uchar bf_data[255];
	int bfdata_len;
	ullong bf_time;
};

typedef struct expression{
	int symbol;
	int parameteres_len;
	double parameteres[10];
}Expression;


typedef struct condition{
	int byte;
	Expression e;
}Condition;

struct singleRule{
	Expression length;
	int conditionLength;
	Condition con[10];        //can be replaced by list too
};


struct formatRule{
	//Expression length;
	unsigned char serviceID;
	int types;
	struct singleRule type[15];    //can be replaced by list

	struct formatRule *next;
};
#endif

typedef struct expression{
	int symbol;
	int parameteres_len;
	double parameteres[10];
}Expression;


typedef struct condition{
	int byte;
	Expression exp;

    struct condition *next;
}Condition;

struct vehicleState{
    int state_type;
    Expression exp;

    struct vehicleState *next;
};

struct singleRule{
	Expression length;
	int conditionLength;
	Condition *con;        //can be replaced by list too

    struct singleRule *next;
};


struct formatRule{
	//Expression length;
	uchar serviceID;
	int types;
	struct singleRule *hrule;    //can be replaced by list

	//struct formatRule *next;
};

struct permissionRule{
    uchar serviceID;
    int number;

    struct singlePermission *head_permission;

};

struct singlePermission{
    struct condition *head_rule;

    struct vehicleState *head_state;

    int decision;

    struct singlePermission *next;
};

struct vehicleService{
	uchar serviceID;
	ullong time;
};

typedef struct splitString{
    char* res;
    char* remain;
}SpString;

//record the suituation of the start of UDS or KWP2000
struct startDiagnosis {
	int status;
	ullong startTime;
	uint can_id;
    uchar session_type;
    int error_code;
};

struct multipleFrame{
	uchar bf_option;
	uchar bf_seq;
	int bf_res; //if before frames have wrong, after ignore
	int multiple_num;
	uchar bf_data[1024];
	int bfdata_len;
	ullong bf_time;
};

int internalPortForward(void);
int externalPortForward(void);

void initFilterHookTable();
void startFrowardMode();
void freeFilterHookTable();

void initFormatRule();
#endif
