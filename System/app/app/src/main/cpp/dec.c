#include "wb_lai_massey.h"
#include "wb_lai_massey_dec.h"
#include "lm_lib.h"
#include "dec.h"
#include <stdlib.h>

uint_8 mtable[STCOUNT][STCOUNT];

void gen_mtable() {
	int i, j, t, tt, k;

	for (i = 0; i < STCOUNT; i++) {
		for (j = 0; j < STCOUNT; j++) {
			t = i & j;
			tt = 0;
			for (k = 0; k < PTLTH; k++) {
				tt ^= (t & 1);
				t = t >> 1;
			}
			mtable[i][j] = tt;
		}
	}
}


uint_8 *mx32_mult_uint32(uint_32 *mx, uint_8 *vec) {
	uint_8 *rslt = (uint_8*)malloc(PTCOUNT * sizeof(uint_8));
	int i, j, t, p;
	for (i = 0; i < PTCOUNT; i++) {
		rslt[PTCOUNT - 1 - i] = 0;
		for (j = 0; j < PTLTH; j++) {
			t = 0; 
			for (p = 0; p < 8; p++) {
				t ^= mtable[vec[p]][(mx[i * 4 + j] >> (p * 4)) & 0xF];
			}
			rslt[PTCOUNT - 1 - i] = (rslt[PTCOUNT - 1 - i] << 1) | t;
		}
	}
	return rslt;
}


uint_8 *F(uint_8 *vec) {
	uint_8 *rslt;
	int i;

	for (i = 0; i < 8; i++) {
		vec[i] = sb(THETA[i], vec[i]);
	}
	rslt = mx32_mult_uint32(DELTA, vec);
	return rslt;
}

void lm_dec(uint_8 *ciphertext, uint_8 *plaintext)
{
	int i, r;
	uint_8 x[PTCOUNT], y[PTCOUNT], s[PTCOUNT];
	gen_mtable();

	for (i = 0; i < PTCOUNT; i++) {	
		x[i] = y[i] = 0; 
	}
	split(ciphertext, x);
	split(&(ciphertext[4]), y);
	
	for (r = ROUND - 1; r >= 0; r--) {
		uint_8 *tempx = (uint_8*)malloc(PTCOUNT * sizeof(uint_8));
		for (i = 0; i < PTCOUNT; i++) { tempx[i] = x[i]; }
		ori(tempx, x);
		for (i = 0; i < PTCOUNT; i++) {
			x[i] = sb(Asi[((r + ROUND) << PTLOG) | i], x[i]);
			y[i] = sb(Asi[(r << PTLOG) | i], y[i]);
			s[i] = x[i] ^ y[i];
		}
		uint_8 *l = mx32_mult_uint32(Li, s);
		uint_8 *f = F(l);
		for (int i = 0; i < PTCOUNT; i++) {
			x[i] ^= f[i];
			y[i] ^= f[i];
		}
		uint_8 *xr = mx32_mult_uint32(Li, x);
		uint_8 *yr = mx32_mult_uint32(Li, y);
		for (int i = 0; i < PTCOUNT; i++) {
			x[i] = sb(Hi[i], xr[i]);
			y[i] = sb(Gi[i], yr[i]);
		}
		free(l); free(f); free(xr); free(yr);
	}
	
	join(plaintext, x);
	join(&(plaintext[4]), y);
}