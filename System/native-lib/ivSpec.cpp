// ivSpec.capp
// This file focus loading the .dbc file, and then generating requests and parsign responses

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <utils/Log.h>
#include <math.h>
#include "ivSpec.h"
#include "dbcSpec.h"
#include "log.h"
//#include <sys/can.h>
//#include <android/log.h>
#include "canitf.h"
//
extern struct sysRunningStatus *sys_state;


struct IVCxtInfo *ack_info;
struct IVReqInfo *all_req_info = NULL;
struct IVReqInfo *used_req_info = NULL;
struct IVSysInfo *used_sys_list = NULL;



//function ===================================================

void createNewDBCItem(struct DBCItem* dbc_item_list){
	printf("start create obj\n");
	if (dbc_item_list==NULL){
				dbc_item_list = (DBCItem *) malloc(sizeof(DBCItem));
				dbc_item_list->msgName[0]='a';
				dbc_item_list->msgName[1]='\0';
				(*dbc_item_list).next = NULL;
				printf("Raw list is null, direct save\n");
	}
	else{
				struct DBCItem* dbc_item_list_tmp = dbc_item_list;
				while ((*dbc_item_list_tmp).next!=NULL){
					printf("Find one item, move next\n");
					dbc_item_list_tmp = (*dbc_item_list_tmp).next;
				}
				DBCItem* tempDBCItem = (DBCItem *) malloc(sizeof(DBCItem));
				(*tempDBCItem).next = NULL;
				(*dbc_item_list_tmp).next = tempDBCItem;
				(*dbc_item_list_tmp).signalNumber = 0;
	}
	printf("start create obj success\n");
}

void storeDBCItemIdNameSize(struct DBCItem* dbc_item_list, char* msgIdSrc, char* msgNameSrc, char* msgSizeSrc, char* msgTransmitterSrc){
		//printf("start save info");
		// find the last DBC item obj
		struct DBCItem *dbc_item_list_tmp = dbc_item_list;
		while ((*dbc_item_list_tmp).next != NULL)
		{
			dbc_item_list_tmp = (*dbc_item_list_tmp).next;
			//printf("move next\n");
		}
		
		// store the information to the new obj
		strcpy((*dbc_item_list_tmp).msgId, msgIdSrc);
		strcpy((*dbc_item_list_tmp).msgName, msgNameSrc);
		strcpy((*dbc_item_list_tmp).msgSize, msgSizeSrc);
		//printf("Store msg name, id, size success\n");
		//printf("save info:%s\n", (*dbc_item_list_tmp).msgName);
		strcpy((*dbc_item_list_tmp).msgTransmitter, msgTransmitterSrc);
}

void storeSignalStartBit_Size_Factor_Offset_Unit(struct DBCItem* dbc_item_list, \
		char* signalName, char* signalStartBit, char* signalSize, char* signalFactor, \
		char* signalOffset, char* signalUnit, char signalFormat, char signalSigned){
		// find the last DBC item obj
		struct DBCItem *dbc_item_list_tmp = dbc_item_list;
		while (dbc_item_list_tmp->next)
		{
			dbc_item_list_tmp = dbc_item_list_tmp->next;
			//printf("move next\n");
		}
        //ALOGD("LYY3 signal: %d ",dbc_item_list_tmp->signalNumber);
        dbc_item_list_tmp->signalNumber = 0;
		// if item has extra space to store signal, save it
		if(dbc_item_list_tmp->signalNumber < MAX_SIGNAL_NUM){
			int signalIndex = dbc_item_list_tmp->signalNumber;
            //ALOGD("LYY3  index:%d",signalIndex);
            
			dbc_item_list_tmp->signalNameList[signalIndex] = (char*) malloc(strlen(signalName)+1);
            memset(dbc_item_list_tmp->signalNameList[signalIndex],0,strlen(signalName)+1);
			strcpy(dbc_item_list_tmp->signalNameList[signalIndex], signalName);
            //ALOGD("LYY6 %d , %d , %s",sizeof(signalName),strlen(signalName),dbc_item_list_tmp->signalNameList[signalIndex]);

			dbc_item_list_tmp->signalStartBitList[signalIndex] = (char*) malloc(strlen(signalStartBit)+1);
            memset(dbc_item_list_tmp->signalStartBitList[signalIndex],0,strlen(signalStartBit)+1);
			strcpy(dbc_item_list_tmp->signalStartBitList[signalIndex], signalStartBit);
			//ALOGD("LYY3 2");

			dbc_item_list_tmp->signalSizeList[signalIndex] = (char*) malloc(strlen(signalSize)+1);
            memset(dbc_item_list_tmp->signalSizeList[signalIndex],0,strlen(signalSize)+1);
			strcpy(dbc_item_list_tmp->signalSizeList[signalIndex], signalSize);

            //ALOGD("LYY3 3");
			dbc_item_list_tmp->signalFactorList[signalIndex] = (char*) malloc(strlen(signalFactor)+1);
            memset(dbc_item_list_tmp->signalFactorList[signalIndex],0,strlen(signalFactor)+1);
			strcpy(dbc_item_list_tmp->signalFactorList[signalIndex], signalFactor);

            //ALOGD("LYY3 4");
			dbc_item_list_tmp->signalOffsetList[signalIndex] = (char*) malloc(strlen(signalOffset)+1);
            memset(dbc_item_list_tmp->signalOffsetList[signalIndex],0,strlen(signalOffset)+1);
			strcpy(dbc_item_list_tmp->signalOffsetList[signalIndex], signalOffset);

            //ALOGD("LYY3 5:sizeof(signalUnit):",strlen(signalUnit));
			dbc_item_list_tmp->signalUnitList[signalIndex] = (char*) malloc(strlen(signalUnit)+1);
            memset(dbc_item_list_tmp->signalUnitList[signalIndex],0,strlen(signalUnit)+1);
			strcpy(dbc_item_list_tmp->signalUnitList[signalIndex], signalUnit);
			
            //ALOGD("LYY3 6");
            //memcpy(dbc_item_list_tmp->signalFormat[signalIndex],signalFormat,sizeof(signalFormat));
            //memcpy(dbc_item_list_tmp->signalSignedOrUnsigned[signalIndex],signalSigned,sizeof(signalSigned));
            dbc_item_list_tmp->signalFormat[signalIndex] = signalFormat;
			dbc_item_list_tmp->signalSignedOrUnsigned[signalIndex] = signalSigned;

			dbc_item_list_tmp->signalNumber = dbc_item_list_tmp->signalNumber + 1;
		}
		//printf("start save info 2");
}

void printDBCItemInfo(struct DBCItem *dbc_item_list){
	struct DBCItem *dbc_item_list_tmp = dbc_item_list;
	int num = 0;
	printf("==========\n");
	while (dbc_item_list_tmp != NULL)
	{
		num = num + 1;
		printf("\t> Msg name:%s\t", (*dbc_item_list_tmp).msgName);
		printf("Msg   id:%s\t", (*dbc_item_list_tmp).msgId);
		printf("Msg size:%s\n", (*dbc_item_list_tmp).msgSize);
		printf("Msg transmitter:%s\n",  (*dbc_item_list_tmp).msgTransmitter);
		if(dbc_item_list_tmp->signalNumber>0){
			for(int index = 0; index < dbc_item_list_tmp->signalNumber; index = index+1){
                printf("LYY3:\tSignal startbit:%s, size:%s, format:%c, signedOrNot:%c, factor:%s, offset:%s, unit:%s\n", dbc_item_list_tmp->signalStartBitList[index],\
										dbc_item_list_tmp->signalSizeList[index],
										dbc_item_list_tmp->signalFormat[index],
										dbc_item_list_tmp->signalSignedOrUnsigned[index],
										dbc_item_list_tmp->signalFactorList[index],
										dbc_item_list_tmp->signalOffsetList[index],
										dbc_item_list_tmp->signalUnitList[index]);
			}
		}
		if(dbc_item_list_tmp->next!=NULL){
			dbc_item_list_tmp = (*dbc_item_list_tmp).next;
		}
		else{
			break;
		}
	}
	printf("\t%d dbc items in total\n", num);
	printf("==========\n");
}

int hexStr2int(char *hex) {
    int val = 0;
    //ALOGD("LYY2 HEX2 %s",hex);
    while (*hex) {
        // get current character then increment
        char byte = *hex++; 
		if(byte=='\0'){
			break;
		}
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
	//printf("\t%s ==> %d\n", hex, val);
    return val;
}

bool stringEqual(char* str1, char* str2){
	int str1Len = strlen(str1);
	int str2Len = strlen(str2);
	if (str1Len!=str2Len){
		return 0;
	}
	for(int i=0; i<str1Len; i++){
		if (str1[i] == str2[i]){
			continue;
		}
		else if(str1[i]=='A' && str2[i]=='a'){
			continue;
		}
		else if(str1[i]=='B' && str2[i]=='b'){
			continue;
		}
		else if(str1[i]=='C' && str2[i]=='c'){
			continue;
		}
		else if(str1[i]=='D' && str2[i]=='d'){
			continue;
		}
		else if(str1[i]=='E' && str2[i]=='e'){
			continue;
		}
		else if(str1[i]=='F' && str2[i]=='f'){
			continue;
		}
		else if(str2[i]=='A' && str1[i]=='a'){
			continue;
		}
		else if(str2[i]=='B' && str1[i]=='b'){
			continue;
		}
		else if(str2[i]=='C' && str1[i]=='c'){
			continue;
		}
		else if(str2[i]=='D' && str1[i]=='d'){
			continue;
		}
		else if(str2[i]=='E' && str1[i]=='e'){
			continue;
		}
		else if(str2[i]=='F' && str1[i]=='f'){
			continue;
		}
		else{
			return 0;
		}
	}
	return 1;
}


void convertUnCharToStr(char* str, unsigned char* UnChar, int UnCharSize)
{
	int i = 0;
	for(i = 0; i < 2; i++)
	{
		sprintf(str + i * 2, "%02x", UnChar[i]);
	}
}

//TODO multiply 
bool funParResposne(DBCItem* currentDBCItem, unsigned char* responseMsg,int len, struct IVCxtInfo* ctx_info){
	// leverage the dbc item information to parse response
	// check the DID exist in response or not
    // 62 11 11 AA AA  len = 5

	if(len<8){ 						// signle frame: First half byte is 0x0.
		if(responseMsg[0]==0x62){ // second byte is 0x62
			// convert unsigned char DID to char DID
			char responseDIDStr[5]={0};
			unsigned char responseDID[3] = {responseMsg[1], responseMsg[2],0};
			convertUnCharToStr(responseDIDStr, responseDID, 2);
            //ALOGD("LYY3 responseDIDStr %s",responseDIDStr);
            //ALOGD("LYY3 msgId %s ",currentDBCItem->msgId);
			// match DID
			if(stringEqual(responseDIDStr, currentDBCItem->msgId)==1){
				//ALOGD("LYY3 Match DID success, start parse response\n");
				ctx_info->ack_len = strlen(responseMsg);
                //ALOGD("LYY3 length:%d", ctx_info->ack_len);
				memcpy(ctx_info->ack_data, responseMsg, strlen(responseMsg));	// save the response msg in ctx_info

				// Parse each signal
				int signalNum = currentDBCItem->signalNumber;
                //ALOGD("LYY3 signalNum: %d",signalNum);
				for(int sigIndex=0; sigIndex<signalNum; sigIndex++){
					char* sigName = currentDBCItem->signalNameList[sigIndex];
					char* sigStartBit = currentDBCItem->signalStartBitList[sigIndex];
					char* sigSize = currentDBCItem->signalSizeList[sigIndex];
					char* sigFactor = currentDBCItem->signalFactorList[sigIndex];
					char* sigOffset = currentDBCItem->signalOffsetList[sigIndex];
					char* sigUnit = currentDBCItem->signalUnitList[sigIndex];
					char sigFormat = currentDBCItem->signalFormat[sigIndex];
					char signedOrNot = currentDBCItem->signalSignedOrUnsigned[sigIndex];
                    
					
					int valueFieldInt = 0;
					int valueFirstBit = 0;
					if(sigFormat=='0'){ 	// Motorola format (Big endian)
						int valueStartBitIndex = atoi(sigStartBit)-7;
						int valueEndBitIndex = valueStartBitIndex + atoi(sigSize);
						printf("Signal start bit:%d, end bit:%d\t\n",valueStartBitIndex, valueEndBitIndex);
						// the response message is hex string
						int valueStartIndex = valueStartBitIndex/8;
						int valueEndIndex = valueEndBitIndex/8;
						for(int j=valueStartIndex; j<valueEndIndex; j++){
							valueFieldInt = valueFieldInt<<8;
							valueFieldInt = valueFieldInt + responseMsg[j];
							//printf("\t>Value field:%d, resp byte:%02x\n", valueFieldInt, responseMsg[j]);
						}

						valueFirstBit = responseMsg[valueStartIndex] & 0x80;
						//printf("Value Field: %d, first byte is one %d\n", valueFieldInt, valueFirstBit);
					}
					else if(sigFormat=='1'){ // Intel format (Little endian)
					}
					
					if(signedOrNot=='+'){ 		// field value is unsigned
						ALOGD("LYY_YL :Field value:%d", valueFieldInt);
					}
					else if(signedOrNot=='-'){	// field value is signed
						if(valueFirstBit != 0){
								int valueFieldLen = atoi(sigSize);
								int totalValue = 1<<(valueFieldLen);
								//printf("\tValue field size:%d\n", valueFieldLen);
								//printf("\tNegative field value \t %d, %d\n", valueFieldLen, totalValue);
								valueFieldInt = valueFieldInt - totalValue;
						}
						printf("Field value:%d", valueFieldInt);
					}
					//ALOGD("LYY3 position 0");	
					// calculate the real sensor value: sensor_field * factor + offset = result
                    ALOGD("LYY_YL %s:",sigFactor);
                    ALOGD("LYY_YL %s:",sigOffset);
					float factorFloatValue = atof(sigFactor);
					float offsetFloatValue = atof(sigOffset);
					float sensorValue = valueFieldInt * factorFloatValue + offsetFloatValue;
                    //ALOGD("LYY3 position 1");
					ctx_info->result = sensorValue;
					strcpy(ctx_info->sensorName, sigName);
					strcpy(ctx_info->sensorUnit, sigUnit);
                    //ALOGD("LYY3 position 2");
					printf("\tSensor field:%d\n", valueFieldInt);
					ALOGD("LYY_YL \tSensor %s: %f %s\n", sigName, sensorValue, sigUnit);
				}
				return true;	// All signal are processed
			}
		}
	}
	else if(responseMsg[0]==0x62){		// process the payload transmitted by multi-frame
			// match DID
			// 62/18/12/00/04/13/87/01/
			//BO_ 6162 Steering_Angle: 8 0712,077C
 			//SG_ Steering_Angle : 39|16@0- (0.1,0) [-100,100] "" EON
			
			// convert unsigned char DID to char DID
			char responseDIDStr[5]={0};
			unsigned char responseDID[3] = {responseMsg[1], responseMsg[2],0};
			convertUnCharToStr(responseDIDStr, responseDID, 2);
            //ALOGD("LYY3 responseDIDStr %s",responseDIDStr);
            //ALOGD("LYY3 msgId %s ",currentDBCItem->msgId);
			// match DID
			if(stringEqual(responseDIDStr, currentDBCItem->msgId)==1){
				//ALOGD("LYY3 Match DID success, start parse response\n");
				ctx_info->ack_len = strlen(responseMsg);
                //ALOGD("LYY3 length:%d", ctx_info->ack_len);
				memcpy(ctx_info->ack_data, responseMsg, strlen(responseMsg));	// save the response msg in ctx_info

				// Parse each signal
					
				int signalNum = currentDBCItem->signalNumber;
                //ALOGD("LYY3 signalNum: %d",signalNum);
				for(int sigIndex=0; sigIndex<signalNum; sigIndex++){
					char* sigName = currentDBCItem->signalNameList[sigIndex];
					char* sigStartBit = currentDBCItem->signalStartBitList[sigIndex];
					char* sigSize = currentDBCItem->signalSizeList[sigIndex];
					char* sigFactor = currentDBCItem->signalFactorList[sigIndex];
					char* sigOffset = currentDBCItem->signalOffsetList[sigIndex];
					char* sigUnit = currentDBCItem->signalUnitList[sigIndex];
					char sigFormat = currentDBCItem->signalFormat[sigIndex];
					char signedOrNot = currentDBCItem->signalSignedOrUnsigned[sigIndex];
                    
					// 62/18/12/00/04/13/87/01/
					//BO_ 6162 Steering_Angle: 8 0712,077C
 					//SG_ Steering_Angle : 39|16@0+ (0.005,-200.0) [-360,360] "" EON

					int valueFieldInt = 0;
					int valueFirstBit = 0;
					if(sigFormat=='0'){ 	// Motorola format (Big endian)
						int valueStartBitIndex = atoi(sigStartBit)-7;
						int valueEndBitIndex = valueStartBitIndex + atoi(sigSize);
						printf("Signal start bit:%d, end bit:%d\t\n",valueStartBitIndex, valueEndBitIndex);
						// the response message is hex string
						int valueStartIndex = valueStartBitIndex/8;
						int valueEndIndex = valueEndBitIndex/8;
						for(int j=valueStartIndex; j<valueEndIndex; j++){
							valueFieldInt = valueFieldInt<<8;
							valueFieldInt = valueFieldInt + responseMsg[j];
							//ALOGD("LYY_YL\t>Value field:%d, resp byte:%02x\n", valueFieldInt, responseMsg[j]);
						}
                        //ALOGD("LYY_YL Value Field:%d \n",valueFieldInt);

						valueFirstBit = responseMsg[valueStartIndex] & 0x80;
						//printf("Value Field: %d, first byte is one %d\n", valueFieldInt, valueFirstBit);
					}
					else if(sigFormat=='1'){ // Intel format (Little endian)
					}
					
					if(signedOrNot=='+'){ 		// field value is unsigned
						//ALOGD("LYY001 :Field value:%d", valueFieldInt);
					}
					else if(signedOrNot=='-'){	// field value is signed
						if(valueFirstBit != 0){
								int valueFieldLen = atoi(sigSize);
								int totalValue = 1<<(valueFieldLen);
								//printf("\tValue field size:%d\n", valueFieldLen);
								//printf("\tNegative field value \t %d, %d\n", valueFieldLen, totalValue);
								valueFieldInt = valueFieldInt - totalValue;
						}
						printf("Field value:%d", valueFieldInt);
					}
					//ALOGD("LYY3 position 0 (multi-frame)");	
					// calculate the real sensor value: sensor_field * factor + offset = result
                    //ALOGD("LYY_YL %s (multi-frame):",sigFactor);
                    //ALOGD("LYY_YL %s (multi-frame):",sigOffset);
					float factorFloatValue = atof(sigFactor);
					float offsetFloatValue = atof(sigOffset);
					float sensorValue = valueFieldInt * factorFloatValue + offsetFloatValue;
                    //ALOGD("LYY3 position 1 (multi-frame)");
					ctx_info->result = sensorValue;
					strcpy(ctx_info->sensorName, sigName);
					strcpy(ctx_info->sensorUnit, sigUnit);
                    //ALOGD("LYY3 position 2 (multi-frame)");
					//printf("\tSensor field (multi-frame):%d\n", valueFieldInt);
					//printf("\tSensor %s (multi-frame): %f %s\n", sigName, sensorValue, sigUnit);
                    ALOGD("LYY_YL Sensor is %s : %f\n",sigName, sensorValue);
				}
				return true;	// All signal are processed
			}
	}
	return false;	// Cannot match DID, do not parse
}

unsigned char* funGenRequest(char* DIDStr,struct IVReqInfo* req_info){
    ALOGD("LYY2 :Generate Request for DID:%s\n", DIDStr);
	// check the DID length: if length is shorter than 2 bytes, ignore
	if(DIDStr==NULL || strlen(DIDStr)<3){
		return NULL;
	}
	unsigned char *requestMsg = req_info->req_data;
	req_info->req_len = 3;
	requestMsg[0] = 0x03; 
	requestMsg[1] = 0x22;
	//requestMsg[2] = '2'; requestMsg[3] = '2';
	
	//char DIDPart1[2]={0};
    char DIDPart1[3]={0};
    memcpy(DIDPart1,DIDStr,2);
	//DIDPart1[0] = DIDStr[0];
	//DIDPart1[1] = DIDStr[1];
	requestMsg[2] = hexStr2int(DIDPart1);
    //ALOGD("LYY2 SB: %d ",requestMsg[2]);
    //ALOGD("LYY2 SB: %c ",DIDStr[0]);
    
	//char DIDPart2[2]={0};
    char DIDPart2[3]={0};
    memcpy(DIDPart2,DIDStr+2,2);
	//DIDPart2[0] = DIDStr[2];
	//DIDPart2[1] = DIDStr[3];
	requestMsg[3] = hexStr2int(DIDPart2);

	for(int i=4; i<8; i++){
		requestMsg[i] = 0x00;
	}
	//req_info->req_data = requestMsg;
	//memcpy(req_info->req_data,requestMsg,strlen(requestMsg));
	
	return requestMsg;
}



// ============================================================


char* readFile(char *filename)
{
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");
    if (handler)
    {
       // Seek the last byte of the file
       fseek(handler, 0, SEEK_END);
       // Offset from the first to the last byte, or in other words, filesize
       string_size = ftell(handler);
       // go back to the start of the file
       rewind(handler);

       // Allocate a string that can hold it all
       buffer = (char*) malloc(sizeof(char) * (string_size + 1) );

       // Read it all in one operation
       read_size = fread(buffer, sizeof(char), string_size, handler);

       // fread doesn't set it so put a \0 in the last position
       // and buffer is now officially a string
       buffer[string_size] = '\0';

       if (string_size != read_size)
       {
           // Something went wrong, throw away the memory and set
           // the buffer to NULL
           free(buffer);
           buffer = NULL;
       }
       // Always remember to close the file.
       fclose(handler);
    }
    return buffer;
}

/**
* check if str starts with pre or not
**/
bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

/**
 * if the string starts with space (" "), remove the space character
 * **/
char *stringSpaceTrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

static char dbc_path[MAX_FILE_PATH];

struct DBCItem *dbc_item_list = NULL; // The list storing all the dbc items

/**
 * The function that parses the DBC file and initializes the dbc item lists
 */
int initSpec(char* dbc_file) {
	int item_num = 0;

    //req_info is the list of all request Message we need
    //req_info = (IVReqInfo*) malloc(sizeof(struct IVReqInfo));
    ack_info = (IVCxtInfo*) malloc(sizeof(struct IVCxtInfo));

	if(strlen(dbc_file) > MAX_FILE_PATH)
		return 0;
	strcpy(dbc_path, dbc_file);

	// TODO: Load the .bdc file;
	// 1) Each item in it is stored in as struct;
	// 2) The structs can be linke using list structure or array
	char* dbcFileContent = readFile(dbc_file);
	dbc_item_list = NULL;
	//printf(dbcFileContent);
	
	// split the file content into multi-parts (use "\n")
    char* line = strtok(dbcFileContent, "\n"); 
    // Keep printing tokens while one of the delimiters present in str[]. 

    //ALOGD("LYY7 POS1");
    while (line != NULL) { 
		//printf("Line:%s\n", line);
        line = stringSpaceTrim(line);
       // ALOGD("LYY7 POS2");
		if (prefix(LABEL_MESSAGE, line)){
			printf("Message def:%s\n", line);
			
			char msgIdName[128]={0};
			char msgId[10]={0};
			char msgName[128]={0};
			char msgTransmitter[20]={0};

			getStringBeforeColon(line, msgIdName);
			getSecondSubstr_AfterSplitBySpace(msgIdName, msgId);
			// transform int string ID to hex string before saving
			int id = atoi(msgId);
			char hexIdVal[5];
			sprintf(hexIdVal,"%0x",id);
			printf("Hex ID:%s\n", hexIdVal);
			strcpy(msgId, hexIdVal);

			printf("\tMsg id:%s\n",msgId);
			getThirdSubstr_AfterSplitBySpace(msgIdName, msgName);
			printf("\tMsg name:%s\n",msgName);
            printf("======================sb===============%s\n",msgIdName);
			char msgSizeTransmitter[128]={0};
			char msgSize[10]={0};
			getStringAfterColon(line, msgSizeTransmitter);
			removeSpacePrefixOfString(msgSizeTransmitter);
			getFirstSubstr_AfterSplitBySpace(msgSizeTransmitter, msgSize);
			printf("\tMsg size:%s\n", msgSize);

			getSecondSubstr_AfterSplitBySpace(msgSizeTransmitter, msgTransmitter);
           // ALOGD("LYY7 POS3");

			if (dbc_item_list==NULL){
				dbc_item_list = (DBCItem *) malloc(sizeof(DBCItem));
                //memset((void*)dbc_item_list,0,sizeof(struct DBCItem));
                //ALOGD("LYY7 POS5");
				dbc_item_list->msgName[0]='a';
				dbc_item_list->msgName[1]='\0';
                dbc_item_list->signalNumber = 0;
				(*dbc_item_list).next = NULL;
				printf("Raw list is null, direct save\n");
               // ALOGD("LYY7 POS4");
			}
			else{
               // ALOGD("LYY7 POS6");
				struct DBCItem* dbc_item_list_tmp = dbc_item_list;
				while ((*dbc_item_list_tmp).next!=NULL){
					//printf("Find one item, move next\n");
					dbc_item_list_tmp = (*dbc_item_list_tmp).next;
				}
               // ALOGD("LYY7 POS9");
				struct DBCItem *tempDBCItem = (struct DBCItem *) malloc(sizeof(DBCItem));
                memset((void *)tempDBCItem,0,sizeof(struct DBCItem));

				(*tempDBCItem).next = NULL;
				(*dbc_item_list_tmp).next = tempDBCItem;
                ALOGD("LYY7 POS10");
			}
            ALOGD("LYY7 %s # %s # %s # %s",msgId,msgName,msgSize,msgTransmitter);
			storeDBCItemIdNameSize(dbc_item_list, msgId, msgName, msgSize,msgTransmitter);
			item_num = item_num + 1;
            ALOGD("LYY7 POS11");
		}
		else if (prefix(LABEL_SIGNAL, line)){
			printf("Signal def:%s\n", line);
            ALOGD("LYY7 POS8");
			
            //LYY3
			char tmpInfo[128]={0};
			char signalName[128]={0};
			getStringBeforeColon(line, tmpInfo);
			getSecondSubstr_AfterSplitBySpace(tmpInfo, signalName);
			//ALOGD("LYY3\tSignal name:%s\n", signalName);
			
			char signalStartBit[10]={0};
			getStringAfterColon(line, tmpInfo);
			getFirstSubstr_AfterSplitByChar(tmpInfo, '|', signalStartBit);
			removeSpacePrefixOfString(signalStartBit);
           // ALOGD("LYY3\tSignal start bit:%s\n", signalStartBit);
			
			char signalSize[10]={0};
			getFirstSubstr_AfterSplitByChar(line, '@', tmpInfo);
			getSecondSubstr_AfterSplitByChar(tmpInfo, '|', signalSize);
		//	ALOGD("LYY3\tSignal size:%s\n",signalSize);

			char signalFormat_Signed[10]={0};
			char signalFormat;
			char signalSignedOrUnsigned;
			getSecondSubstr_AfterSplitByChar(line, '@', tmpInfo);
			signalFormat = tmpInfo[0];
			signalSignedOrUnsigned = tmpInfo[1];

			char signalFactor[10]={0};
			char signalOffset[10]={0};
			getSecondSubstr_AfterSplitByChar(line, '(', tmpInfo);
			getFirstSubstr_AfterSplitByChar(tmpInfo, ',', signalFactor);
			getFirstSubstr_AfterSplitByChar(line, ')', tmpInfo);
			getSecondSubstr_AfterSplitByChar(tmpInfo, ',', signalOffset);
		//	ALOGD("LYY3\tSignal  scale:%s\n",signalFactor);
			printf("\tSignal offset:%s\n",signalOffset);

			char signalUnit[10]={0};
			getSecondSubstr_AfterSplitByChar(line, ']', tmpInfo);
			removeSpacePrefixOfString(tmpInfo);
			getFirstSubstr_AfterSplitByChar(tmpInfo, ' ', signalUnit);
		//	ALOGD("LYY3\tSignal unit:%s\n",signalUnit);
			storeSignalStartBit_Size_Factor_Offset_Unit(dbc_item_list, signalName, signalStartBit, signalSize, signalFactor, signalOffset, signalUnit, signalFormat, signalSignedOrUnsigned);
		}
		else if (prefix(LABEL_COMMENT, line)){
			//printf("Comment def: %s\n", line);
		}

		line = strtok(NULL, "\n"); 
	} 
    //printf("\nNAME:%s\n",dbc_item_list->msgName);
	//printDBCItemInfo(dbc_item_list);
    //printf("first end");
    ALOGD("LYY7 end;");
	return item_num;
}

/**
 * Query the DBCItem according to the item name (i.e. vec)
 */
struct DBCItem* getDBCItem(char* vec_name) {
	struct  DBCItem *tmp = dbc_item_list;
	while(tmp) {
		if(strcmp(vec_name, tmp->vec_name) == 0)
			return tmp;
	}
	return NULL;
}

/**
 * Generate request according to the DBCItem
 */
void genIVCxtReq(struct DBCItem* dbc_item) {
    struct IVReqInfo *req = NULL;
    struct IVReqInfo *tmp = NULL;
    //STRUct IVReqInfo *all_req_info = NULL;
    printf("hello lyy");
    
	// TODO: Initialize cxt_info according to dbd_item (except the response info)
	DBCItem* tmp_dbc_item_list = dbc_item;
	while (tmp_dbc_item_list != NULL)
	{

        req =  (struct IVReqInfo*)malloc(sizeof(struct IVReqInfo));
        memset((void*)req, 0, sizeof(struct IVReqInfo));
    
		// get the information for request
		char* DID = tmp_dbc_item_list->msgId;
		tmp_dbc_item_list->funGenRequest = funGenRequest;
        //req->name = tmp_dbc_item_list->msgName;
        printf("gen NAME:%s\n",tmp_dbc_item_list->msgName);
        memcpy(req->name,tmp_dbc_item_list->msgName,100);
        //unsigned char* reqMsg;
		unsigned char* reqMsg = tmp_dbc_item_list->funGenRequest(DID,req);


		char tmp_req_id[5] = {0};
        char tmp_ack_id[5] = {0};
        memcpy(tmp_req_id,tmp_dbc_item_list->msgTransmitter,4);
        memcpy(tmp_ack_id,tmp_dbc_item_list->msgTransmitter+5,4);
        //printf("\n A:%s     B:%s \n",tmp_req_id,tmp_ack_id);

        req->req_id = (ushort)hexStr2int(tmp_req_id);
        req->ack_id = (ushort)hexStr2int(tmp_ack_id);

        //printf("\n REQID:%u   ACKID:%u\n",req->req_id,req->ack_id);
        //memcpy(req->req_data,reqMsg,8);
        //req->req_len = 8;
		ALOGD("LYY2: Request msg:");
		for(int i=0; i<8; i++){ALOGD("LYY2: %02x ", req->req_data[i]);}
        	printf("\n");
    

        //add to list
        if(all_req_info==NULL){
            printf("first\n");
            all_req_info = req;
        }else{
            printf("second\n");

            tmp = all_req_info;

            printf("lyy\n");
            while(tmp->next){
                printf("1\n");
                tmp = tmp->next;
            }
            
            tmp->next = req;
        }

		tmp_dbc_item_list = tmp_dbc_item_list->next;
	}
	
}

unsigned char* getReq(char* name){
    struct IVReqInfo *tmp_list = NULL;

    printf("!!!!!!!!!!!!!!!!!!!\n");
    tmp_list = used_req_info;
    while(tmp_list){
        if(strcmp(tmp_list->name,name)==0){
            break;
        }
        tmp_list = tmp_list->next;
    }
    if(tmp_list==NULL){
        printf("##################NO SENSOR#################");
        return NULL;
    }
    printf("Request msg:");
	for(int i=0; i<8; i++){
        printf("%02x ", tmp_list->req_data[i]);
    }
    printf("\n");

    return tmp_list->req_data;
}

int addUsedIVCxtReq(char* name){

    struct IVReqInfo *tmp = NULL;
    struct IVReqInfo *req = NULL;

    struct IVReqInfo *tmp_list = all_req_info;
    while(tmp_list){
        //printf("%s/%s\n",tmp_list->name,name);
        if(strcmp(tmp_list->name,name)==0){
            break;
        }
        tmp_list = tmp_list->next;
    }
	if(tmp_list==NULL){
		return -1;//NO this DBC item
	}
	
	ushort this_req_id = tmp_list->req_id;
	struct IVSysInfo *tmp_sys_list = used_sys_list;
	while(tmp_sys_list){
		if(tmp_sys_list->req_id == this_req_id){
			break;
		}

		tmp_sys_list = tmp_sys_list->next;
	}
	if(tmp_sys_list==NULL){
		return -2;//NO THIS SYS
	}
	//add to SYS_REQ_LIST
    req = (struct IVReqInfo*)malloc(sizeof(struct IVReqInfo));
    memset((void*)req, 0, sizeof(struct IVReqInfo));
#if 0 
	//check repeat
	struct IVReqInfo* tmp_req_list = tmp_sys_list->req_list;
	while(tmp_req_list){
		if(strcmp(tmp_req_list->name,name)==0){
			return -3;//repeat
		}
		tmp_req_list = tmp_req_list->next;
	}

#endif
    
    memcpy(req->name,tmp_list->name,100);
    req->req_len = 3;
    memcpy(req->req_data,tmp_list->req_data,100);
    req->req_id = tmp_list->req_id;
    req->ack_id = tmp_list->ack_id;
	req->flag = true;
	req->frequence=0;
	//special rule
	if(strcmp(req->name,"")==0){
		//TODO
		//req->frequence = 
	}
    
    //printf("\nAAAAAAAAAAAAAAAa %u \n",req->req_id);
	
    ALOGD("LYY9: SYS_name:%s",tmp_sys_list->name);
	for(int i=0; i<8; i++){
        ALOGD("LYY1: %02x ", req->req_data[i]);
    }
    //ALOGD("\n");

    if(tmp_sys_list->req_list==NULL){
        tmp_sys_list->req_list = req;
    }else{
        tmp = tmp_sys_list->req_list;
        while(tmp->next){
            tmp = tmp->next;
        }
         tmp->next = req;
    }
    return 1;

}



void saveResultInFile(float result, char* sensorName, char* sensorUnit){
	FILE *fptr;
	fptr = fopen("/sdcard/result.txt","a");
	if(fptr==NULL){
		return;
	}
	fputs("Sensor:", fptr);
	fputs(sensorName, fptr);
	fputc('\t', fptr);
	//fwrite(&result, 1, sizeof(result), fptr);
	fprintf(fptr,"%f ",result);
	fputc('\t', fptr);
	fputs(sensorUnit, fptr);
	fputc('\n',fptr);
	fclose(fptr);
}

/**
 * Parse the response (i.e., ack_data) of cxt_info, and the results stored using
 * double type.
 *
 */

bool parseIVCxtAck(struct IVCxtInfo* cxt_info,unsigned char* testResp,int len) {
	/*
	 * Parse the resposne data
	 * 1) look up the DBCItem structure according to the field #dbc_item#
	 * 2) parse the response by invoking the pasing function pointed by field #funcParResponse#
	 * TODO: the function arguments of the parsing function NEED to be well designed
	 */
	DBCItem* tmp_dbc_item_list = dbc_item_list;
	//ALOGD("LYY3 LIST%d",tmp_dbc_item_list);
	while (tmp_dbc_item_list != NULL && sys_state->run_mode & SYS_MODE_CONTEXT)
	{
		// get the information for request
		tmp_dbc_item_list->funParResposne = funParResposne;
		bool parsedOrNot = tmp_dbc_item_list->funParResposne(tmp_dbc_item_list, testResp, len, cxt_info);
        //ALOGD("LYY3 POS1:%d",parsedOrNot);
		if(parsedOrNot){
			//saveResultInFile(cxt_info->result, cxt_info->sensorName, cxt_info->sensorUnit);
			//ALOGD("LYY3 POS2");
			break;
		}
        //ALOGD("Lyy3 POS3");
		tmp_dbc_item_list = tmp_dbc_item_list->next;
	}
	return true;
}

/**
 * FreemZthe DBCItem list
 */
void finiSpec() {
	struct DBCItem *tmp = dbc_item_list, *next = NULL;
	while(tmp) {
		next = tmp->next;
		free(tmp);
		tmp  = next;
	}
}
#if 0
}
#endif

void initIVSys(){
	used_sys_list = NULL;
	struct IVSysInfo * tmp = NULL;
	struct IVSysInfo * new_node = NULL;
	//SYS_INFO
    int sysno = 7;
	char *SYS_name[] = {"ENGINE","Automatic Transmission","Steering","Electronic central electrical","Driver side door","Instrument panel","Front passenger side door"};
	ushort req_id_list[] = {0x07E0,0x07E1,0x0712,0x070E,0x074A,0x0714,0x074B};
	ushort ack_id_list[] = {0x07E8,0x07E9,0x077C,0x0778,0x07B4,0x077E,0x07B5};
	uchar build_data[8] = {0x02,0x10,0x03,0x00,0x00,0x00,0x00,0x00};
	uchar leave_data[2] ={0xA8,0}; 

	//ALOGD("LYY4 : POS1");
	
	for(int i =0;i<sysno;i++){
		
		new_node = (struct IVSysInfo*)malloc(sizeof(struct IVSysInfo));
		memset((void *)new_node,0,sizeof(struct IVSysInfo));

		//ALOGD("LYY4 position1");
		memcpy(new_node->name,SYS_name[i],strlen(SYS_name[i]));
		new_node->req_id = req_id_list[i];
		new_node->ack_id = ack_id_list[i];
        ALOGD("LYY9 %s",new_node->name);

		//ALOGD("LYY4 position2");
		new_node->build_session_request.id = new_node->req_id;
		new_node->build_session_request.length = 8;
		memcpy(new_node->build_session_request.data,build_data,new_node->build_session_request.length);
		
		//ALOGD("LYY4 position3");
		new_node->leave_session_request.id = new_node->req_id;
		new_node->leave_session_request.length = 1;
		memcpy(new_node->leave_session_request.data,leave_data,1);

		new_node->req_list = NULL;
		//ALOGD("LYY4 POS2");

		if(used_sys_list==NULL){
			used_sys_list = new_node;
		}else{
			tmp = used_sys_list;
			while(tmp->next){
				tmp = tmp->next;
			}
			tmp->next = new_node;
		}

	}
}

void initUDSInfo(){
	ALOGD("Enter %s", __func__);
    int item_num = initSpec("/sdcard/uds.dbc");//import dbc return number
    genIVCxtReq(dbc_item_list);//translate all dbc
	initIVSys();
    
	//add to the list we will used
    //static char *Used_name[] = {"Speed","Acc_Sword","RPM","Accelerator","Brake_Pressure","Lateral_Acc","Vertical_Acc","Vertical_Acc_Raw_Value","Tilt_Angular_Acc_Raw_Value","Auto_Trans_Brake_Pressure","Steering_Angle"};
    static char *Used_name[] = {"Speed","Acc_Sword","RPM","Brake_Pressure","Lateral_Acc","Vertical_Acc","Vertical_Acc_Raw_Value","Tilt_Angular_Acc_Raw_Value","Auto_Trans_Brake_Pressure","Auto_Trans_Accelerator","Steering_Angle"};


	int res = 0;
    for(int i = 0; i < 11; i++){
        res = addUsedIVCxtReq(Used_name[i]);
		// ALOGD("LYY7 INDEXL_%d addresult: %d",i,res);
    }

}
/*
int main() {
	test();
	return 0;
}
*/
