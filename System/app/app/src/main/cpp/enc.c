#include "wb_lai_massey.h"
#include "wb_lai_massey_enc.h"
#include "lm_lib.h"
#include "enc.h"
#include <stdlib.h>
#include <jni.h>
#include <string.h>
#include <android/log.h>


void sigma_box(uint_32 vec, uint_8 *xt, int is_r, int index) {
	uint_8 *xt1 = disassemble(vec);

	if (is_r == 0)
	{
		for (int j = 0; j < PTCOUNT; j++) {
			xt[j] = SIGMA[index | j][((xt[j] << PTLTH) | xt1[j]) >> 1];
		}
	}
	else {
		for (int j = 0; j < PTCOUNT; j++) {
			xt[j] = SIGMAr[index | j][((xt[j] << PTLTH) | xt1[j]) >> 1];
		}
	}

	for (int j = 0; j < PTCOUNT; j++) {
		if (xt1[j] & 1) {
			xt[j] >>= PTLTH;
		}
		else {
			xt[j] &= 0xF;
		}
	}
	free(xt1);
}


char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* cArray = NULL;
    jbyte* byteArray = (*env)->GetStringUTFChars(env, jstr, NULL);
    // __android_log_print(ANDROID_LOG_INFO, "[polyu]", "%s", (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, byteArray), NULL));
    jsize length = (*env)->GetStringUTFLength(env, jstr);
    if (length > 0) {
        int paddingSize = 0;
        if (length % 8 != 0)
            paddingSize = 8 - (length % 8);
        cArray = (char*) malloc(length + paddingSize + 1);
        memset(cArray, '\0', length);
        memcpy(cArray, byteArray, length);
        for (int i = 0; i < paddingSize; i++) {
            cArray[length + i] = 3;
        }
        cArray[length + paddingSize] = '\0';
    }

    return cArray;
}

jstring hexEncode(JNIEnv *env, jbyteArray array) {
    // 1. 数组长度；2. new StringBuilder(); or char[len * 2] 3. char[] -> jstring
    jstring ret = NULL;
    if (array != NULL) {
        //得到数组的长度
        jsize len = (*env)->GetArrayLength(env, array);
        if (len > 0) {
            //存储编码后的字符, +1的原因是考虑到\0
            char chs[len * 2 + 1];
            jboolean b = JNI_FALSE;
            //得到数据的原始数据 此处注意要取b的地址!
            jbyte *data = (*env)->GetByteArrayElements(env, array, &b);
            int index;
            for (index = 0; index < len; index++) {
                jbyte bc = data[index];
                //拆分成高位, 低位
                jbyte h = (jbyte) ((bc >> 4) & 0x0f);
                jbyte l = (jbyte) (bc & 0x0f);
                //把高位和地位转换成字符
                jchar ch;
                jchar cl;

                if (h > 9) {
                    ch = (jchar) ('A' + (h - 10));
                } else {
                    ch = (jchar) ('0' + h);
                }

                if (l > 9) {
                    cl = (jchar) ('A' + (l - 10));
                } else {
                    cl = (jchar) ('0' + l);
                }
                //转换之后拼接
                chs[index * 2] = (char) ch;
                chs[index * 2 + 1] = (char) cl;
            }
            //最后一位置为0
            chs[len * 2] = '\0';
            //释放数组
            (*env)->ReleaseByteArrayElements(env, array, data, JNI_ABORT);
            ret = (*env)->NewStringUTF(env, chs);
        }
    }
    return ret;
}

static void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
    char	ddl,ddh;
    int i;

    for (i=0; i<nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}

JNIEXPORT jstring Java_cn_uprogrammer_sensordatacollect_IPSService_enc(
        JNIEnv *env,
        jobject thiz,
        jstring plaintext) {
    char* plaintextPointer = jstringToChar(env, plaintext);
    //__android_log_print(ANDROID_LOG_INFO, "[polyu]", "%s", (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, plaintextPointer), NULL));
    int plaintextSize = (*env)->GetStringUTFLength(env, (*env)->NewStringUTF(env, plaintextPointer));
    //__android_log_print(ANDROID_LOG_INFO, "[polyu]", "%d", plaintextSize);
    //
    char* cipherTextString = (char*) malloc(plaintextSize * 2 + 1);
    memset(cipherTextString, '\0', plaintextSize * 2 + 1);
    for(int i = 0; i < plaintextSize / 8; i++) {
        char plaintextSegment[8];
        memset(plaintextSegment, '\0', 8);
        memcpy(plaintextSegment, plaintextPointer + i * 8, 8);
        // __android_log_print(ANDROID_LOG_INFO, "[polyu]", "%s", (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, plaintextSegment), NULL));
        char *ciphertext = (char*) malloc(8);
        memset(ciphertext, '\0', 8);
        lm_enc(plaintextSegment, ciphertext);

        // jstring cipherTextStringSegment = (*env)->NewStringUTF(env, ciphertext);
        char* src = ciphertext;
        char* des = (char*) malloc(100);
        HexToStr(des, src, 8);
        //__android_log_print(ANDROID_LOG_INFO, "[polyu]", "%s", (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, des), NULL));
        strcat(cipherTextString, des);
        free(des);
    }

    //
    //__android_log_print(ANDROID_LOG_INFO, "[polyu] output", "%s", (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, cipherTextString), NULL));
    return (*env)->NewStringUTF(env, cipherTextString);
}

void lm_enc(uint_8 *plaintext, uint_8 *ciphertext)
{
	int i, r;
	uint_8 x[PTCOUNT], y[PTCOUNT];
	uint_8 *xt, *yt;
	uint_32 xm, ym;
	uint_64 rm;

	for (i = 0; i < PTCOUNT; i++) {	
		x[i] = y[i] = 0; 
	}
	split(plaintext, x);
	split(&(plaintext[4]), y);

	for (r = 0; r < ROUND; r++) {
		i = PTCOUNT - 1;
		rm = TBox[i][(x[i] << PTLTH) | y[i]];
		xm = rm >> (PTCOUNT * PTLTH);
		ym = rm & 0xFFFFFFFF;
		xt = disassemble(xm);
		yt = disassemble(ym);
		for (i = PTCOUNT - 2; i >= 0; i--) {
			rm = TBox[i][(x[i] << PTLTH) | y[i]];
			xm = rm >> (PTCOUNT * PTLTH);
			ym = rm & 0xFFFFFFFF;
			int is_r, index_x, index_y;
			if (i == 0) {
				is_r = 1;	index_y = r;	index_x = r + ROUND;
			}
			else {
				is_r = 0;	index_y = i - 1;	index_x = i - 1 + PTCOUNT - 2;
			}
			index_x <<= PTLOG;
			index_y <<= PTLOG;
			sigma_box(xm, xt, is_r, index_x);
			sigma_box(ym, yt, is_r, index_y);
		}
		
		for (int i = 0; i < PTCOUNT; i++) {
			//x[i] = xt[i];
			y[i] = yt[i];
		}
		or(xt, x);
	}
	
	join(ciphertext, x);	
	join(&(ciphertext[4]), y);

	free(xt);	
	free(yt);
}
