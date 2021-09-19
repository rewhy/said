#pragma once
#include "round.h"

#define		STCOUNT		16
#define		PTLTH		4
#define		PTCOUNT		8
#define		PTLOG		3
//#define		ROUND		10

// define unsigned type to be compatible with non-C++11 compiler
typedef unsigned char uint_8;
typedef unsigned int uint_32;
typedef unsigned long long uint_64;

typedef struct {
	uint_64 a, b;
}sbox_union;

typedef struct {
	uint_32 r[32];
}mx_32;