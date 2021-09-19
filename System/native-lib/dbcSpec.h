#include <string.h>
#include <stdio.h>

#define LABEL_MESSAGE "BO_ "
#define LABEL_SIGNAL "SG_ "
#define LABEL_COMMENT "CM_ "
#define LABEL_VALUE "VAL"

// split line into two part by ":", return the first one
void getStringBeforeColon(char* str, char* dst){
    int strLen = strlen(str);
    int colonIndex = -1;
    for(int i=0; i<strLen; i++){
        if(str[i]==':'){
            //printf("%d\n",i);
            colonIndex = i;
            break;    
        }
    }

    if (colonIndex != -1){
        for(int i=0; i<colonIndex; i++){
            dst[i] = str[i];
        }
        dst[colonIndex] = '\0';
        //printf("tmpLine:%s\n",tmpLine);
        //return tmpLine;
    }
    
    //return NULL;
}

// split line into two part by ":", return the second one
void getStringAfterColon(char* str, char* dst){
    int strLen = strlen(str);
    int colonIndex = -1;
    for(int i=0; i<strLen; i++){
        if(str[i]==':'){
            //printf("%d\n",i);
            colonIndex = i;
            break;    
        }
    }

    if (colonIndex != -1){
        int j = 0;
        for(int i=colonIndex+1; i<strLen; i++){
            dst[j] = str[i];
            j = j + 1;
        }
        dst[strLen] = '\0';
        //printf("tmpLine:%s\n",tmpLine);
        //return tmpLine;
    }
}

// split line into two part by "　", return the first one
void getFirstSubstr_AfterSplitBySpace(char* strSrc, char* dst){
    int strLen = strlen(strSrc);
    int spaceIndex = -1;
    for(int i=0; i<strLen; i++){
        if(strSrc[i]==' '){
            //printf("%d\n",i);
            spaceIndex = i;
            break;    
        }
    }
    if (spaceIndex != -1){
        for(int i=0; i<spaceIndex; i++){
            dst[i] = strSrc[i];
        }
        dst[spaceIndex] = '\0';
    }
}

// split line into two part by "　", return the second one
void getSecondSubstr_AfterSplitBySpace(char* str, char* dst){
    int strLen = strlen(str);
    int spaceIndex = -1;
    for(int i=0; i<strLen; i++){
        if(str[i]==' '){
            //printf("%d\n",i);
            spaceIndex = i;
            break;    
        }
    }

    if (spaceIndex != -1){
        int j = 0;
        for(int i=spaceIndex+1; i<strLen; i++){
            if(str[i] != ' '){
                dst[j] = str[i];
                j = j + 1;
            }
            else{
                break;
            }
        }
        dst[j] = '\0';
        //printf("tmpLine:%s\n",tmpLine);
        //return tmpLine;
    }
}

// split line into two part by "　", return the third one
void getThirdSubstr_AfterSplitBySpace(char* str, char* dst){
    int strLen = strlen(str);
    int spaceCount = 0;
    int spaceIndex = -1;
    for(int i=0; i<strLen; i++){
        if(str[i]==' '){
            //printf("%d\n",i);
            spaceCount = spaceCount + 1;
            if(spaceCount==2){
                spaceIndex = i;
                break;
            }
        }
    }

    if (spaceIndex != -1){
        int j = 0;
        for(int i=spaceIndex+1; i<strLen; i++){
            if(str[i] != ' '){
                dst[j] = str[i];
                j = j + 1;
            }
            else{
                break;
            }
        }
        dst[j] = '\0';
        //printf("tmpLine:%s\n",tmpLine);
        //return tmpLine;
    }
    else{
        dst[0]='\0';
    }
}

// split line into two part by char, return the first one
void getFirstSubstr_AfterSplitByChar(char* strSrc, char splitChar, char* dst){
    int strLen = strlen(strSrc);
    int spaceIndex = -1;
    for(int i=0; i<strLen; i++){
        if(strSrc[i]==splitChar){
            //printf("%d\n",i);
            spaceIndex = i;
            break;    
        }
    }
    if (spaceIndex != -1){
        for(int i=0; i<spaceIndex; i++){
            dst[i] = strSrc[i];
        }
        dst[spaceIndex] = '\0';
    }
}

// split line into two part by char, return the second one
void getSecondSubstr_AfterSplitByChar(char* str, char splitChar, char* dst){
    int strLen = strlen(str);
    int spaceIndex = -1;
    for(int i=0; i<strLen; i++){
        if(str[i]==splitChar){
            //printf("%d\n",i);
            spaceIndex = i;
            break;    
        }
    }

    if (spaceIndex != -1){
        int j = 0;
        for(int i=spaceIndex+1; i<strLen; i++){
            if(str[i] != splitChar){
                dst[j] = str[i];
                j = j + 1;
            }
            else{
                break;
            }
        }
        dst[j] = '\0';
        //printf("tmpLine:%s\n",tmpLine);
        //return tmpLine;
    }
}

void removeSpacePrefixOfString(char* str){
    int i = 0;
    while (str[i]==' ')
    {
        i = i + 1;
    }
    if (i>0){
        for(int j=i; j<=strlen(str); j++){
            str[j-i] = str[j];
        }
    }
    //printf("length:%d\n", strlen(str));
}

void charCopy(char* hexChar, char* unsignedChar){
    char hexCharList[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    for(int i=0; i<strlen(hexChar); i++){

    }
}
