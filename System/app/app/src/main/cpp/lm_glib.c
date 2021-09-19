#include "wb_lai_massey.h"
#include "lm_glib.h"
#include <stdlib.h>

// 8 * uint_4 --> uint_64
uint_64 make_uint64(uint_8* a) {
	uint_64 rslt = 0;
	int i;
	
	for (i = STCOUNT - 1; i >= 0; i--) {
		rslt = (rslt << PTLTH) | (a[i] & 0xF);
	}
	return rslt;
}


// generate bijection a <-> b
void gen_bijection(int length, uint_8 *a, uint_8 *b) {
	int i, limit, target;
	
	limit = STCOUNT;
	for (i = 0; i < limit; i++) { 
		a[i] = i; 
	}

	for (i = 0; i < limit - 1; i++) {
		target = i + rand() % (limit - i);
		uint_8 temp = a[target];	a[target] = a[i];	a[i] = temp;
		b[a[i]] = i;
	}
	b[a[limit - 1]] = limit - 1;
}


sbox_union gen_sbox() {
	uint_8 a[STCOUNT], b[STCOUNT];
	sbox_union rslt;

	gen_bijection(PTLTH, a, b);
	rslt.a = make_uint64(a);	
	rslt.b = make_uint64(b);
	return rslt;
}


// generate inversible 32*32 matrices
void gen_mx32(mx_32 *a, mx_32 *b) {
	int i, repeat, from, to, type;
	step st;

	for (i = 0; i < 32; i++) { 
		a->r[i] = b->r[i] = 1 << (31 - i);
	}

	repeat = 1024;
	for (i = 0; i < repeat; i++) {
		from = rand() % 32;
		to = rand() % 32;
		while (to == from) { to = rand() % 32; }
		type = rand() % 2;
		st.type[i] = type;	st.from[i] = from; st.to[i] = to;
		
		if (type == 0) {
			a->r[to] ^= a->r[from];
		} else {
			uint_32 temp;
			temp = a->r[to];	a->r[to] = a->r[from];	 a->r[from] = temp;
		}
	}
	for (i = repeat - 1; i >= 0; i--) {
		from = st.from[i];
		to = st.to[i];
		type = st.type[i];
		if (type == 0) {
			b->r[to] ^= b->r[from];
		} else {
			uint_32 temp;
			temp = b->r[to];	b->r[to] = b->r[from];	 b->r[from] = temp;
		}
	}
}


uint_32 mx32_mult_uint4(mx_32 mx, int i, uint_8 vec) {
	uint_32 rslt = 0;
	int r, t;
	
	for (r = 0; r < 32; r++) {
		t = (mx.r[r] >> (i * PTLTH)) & 0xF;
		rslt = (rslt << 1) | mtable[t][vec];
	}
	return rslt;
}