#pragma once

uint_64 make_uint64(uint_8* a);
void gen_bijection(int length, uint_8 *a, uint_8 *b);
sbox_union gen_sbox();
void gen_mx32(mx_32 *a, mx_32 *b);
uint_32 mx32_mult_uint4(mx_32 mx, int i, uint_8 vec);


typedef struct {
	int type[1024], from[1024], to[1024];
}step;

uint_8 mtable[STCOUNT][STCOUNT];