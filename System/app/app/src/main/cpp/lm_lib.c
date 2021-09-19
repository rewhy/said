#include "wb_lai_massey.h"
#include "lm_lib.h"
#include <stdlib.h>
#include <stdio.h>

// 0xAB --> 0x0A | 0x0B
void split(uint_8 *text, uint_8 *x) {
	int i;
	for (i = 0; i < PTCOUNT; i += 2) {
		x[PTCOUNT - 1 - i] = text[i >> 1] >> PTLTH;
	}
	for (i = 1; i < PTCOUNT; i += 2) {
		x[PTCOUNT - 1 - i] = text[i >> 1] & 0xF;
	}
}


// 0x0A | 0x0B -->  0xAB
void join(uint_8 *result, uint_8 *x) {
	int i;
	for (i = 0; i < PTCOUNT; i += 2) {
		result[i >> 1] = x[PTCOUNT - 1 - i] << PTLTH; 
	}
	for (i = 1; i < PTCOUNT; i += 2) {
		result[i >> 1] |= x[PTCOUNT - 1 - i];
	}
}


// uint_32 --> 8 * uint_4
uint_8 *disassemble(uint_32 vec) {
	uint_8 *rslt = (uint_8*)malloc(PTCOUNT * sizeof(uint_8));
	int i;
	for (i = 0; i < PTCOUNT; i++) {
		rslt[i] = (vec >> (i * PTLTH)) & 0xF;
	}

	return rslt;
}

uint_32 assemble(uint_8 *a) {
	uint_32 rslt = 0;
	int i;
	for (i = PTCOUNT - 1; i >= 0; i--) {
		rslt = (rslt << PTLTH) | a[i];
	}
	return rslt;
}


// get uint_4 from uint_64
uint_8 sb(uint_64 sbox, uint_8 vec) {
	uint_8 rslt;
	rslt = (sbox >> (vec * PTLTH)) & 0xF;
	return rslt;
}


// print 8 * uint_8
void lg(uint_8 *x, char c) {
	printf("%c : ", c);
	for (int i = 0; i < 8; i++) {
		printf("%x ", x[7 - i]);
	}
	printf("\n");
}


void or(uint_8 *x, uint_8 *rslt) {
	int i;
	for (i = 0; i < PTCOUNT; i++) {
		rslt[(i + 4) % PTCOUNT] = x[i];
	}
	for (i = 0; i < 4; i++) {
		rslt[i] ^= rslt[i + 4];
	}
}


void ori(uint_8 *x, uint_8 *rslt) {
	int i;
	for (i = 0; i < PTCOUNT; i++) {
		rslt[(i + 4) % PTCOUNT] = x[i];
	}
	for (i = 0; i < 4; i++) {
		rslt[i + 4] ^= rslt[i];
	}
}


int cb3(int x, int y) {
	return (x << PTLOG) | y;
}

int cb4(int x, int y) {
	return (x << PTLTH) | y;
}