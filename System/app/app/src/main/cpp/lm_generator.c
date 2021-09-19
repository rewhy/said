#include "wb_lai_massey.h"
#include "lm_lib.h"
#include "lm_glib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


sbox_union H[PTCOUNT];
sbox_union G[PTCOUNT];
sbox_union Ap[2 * PTCOUNT * PTCOUNT];
sbox_union A[2 * (PTCOUNT - 2) * PTCOUNT];
sbox_union As[2 * ROUND * PTCOUNT];
sbox_union THETA[PTCOUNT];

mx_32 L, Li, DELTA;
uint_8 SIGMA[2 * (PTCOUNT - 2) * PTCOUNT][STCOUNT * STCOUNT / 2];
uint_8 SIGMAr[2 * ROUND * PTCOUNT][STCOUNT * STCOUNT / 2];
uint_64 TBox[PTCOUNT][STCOUNT * STCOUNT];



void gen_tables()
{
	int i, j, t, k, r;
	uint_8 tt;
	mx_32 temp;

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

	srand (time(NULL));
	for (i = 0; i < PTCOUNT; i++) {
		H[i] = gen_sbox();
		G[i] = gen_sbox();
		THETA[i] = gen_sbox();
		for (j = 0; j < PTCOUNT; j++) {
			Ap[(i << PTLOG) | j] = gen_sbox();
			Ap[((i + PTCOUNT) << PTLOG) | j] = gen_sbox();
		}
	}

	for (r = 0; r < ROUND; r++) {
		for (j = 0; j < PTCOUNT; j++) {
			As[(r << PTLOG) | j] = gen_sbox();
			As[((r + ROUND) << PTLOG) | j] = gen_sbox();
		}
	}

	for (i = 0; i < PTCOUNT - 2; i++) {
		for (j = 0; j < PTCOUNT; j++) {
			A[(i << PTLOG) | j] = gen_sbox();
			A[((i + PTCOUNT - 2) << PTLOG) | j] = gen_sbox();
		}
	}

	gen_mx32(&L, &Li);
	gen_mx32(&DELTA, &temp);
}


uint_32 F(int i, uint_8 vec) {
	uint_32 rslt = 0;
	vec = sb(THETA[i].a, vec);
	rslt = mx32_mult_uint4(DELTA, i, vec);
	return rslt;
}


void gen_tbox() {
	int i, v, j;

	for (i = 0; i < PTCOUNT; i++) {
		for (v = 0; v < STCOUNT * STCOUNT; v++) {
			uint_8 x, y, sum;
			uint_32 lx, ly, f, px, py;

			x = v >> PTLTH;
			y = v & 0xF;
			// H, G
			x = sb(H[i].a, x);
			y = sb(G[i].a, y);
			// L
			lx = mx32_mult_uint4(L, i, x);
			ly = mx32_mult_uint4(L, i, y);
			// THETA -> DELTA
			sum = x ^ y;
			f = F(i, sum);
			// XOR
			lx = lx ^ f;
			ly = ly ^ f;
			// SIGMA
			px = py = 0;
			for (j = PTCOUNT - 1; j >= 0; j--) {
				x = (lx >> (j * PTLTH)) & 0xF;
				y = (ly >> (j * PTLTH)) & 0xF;
				px = (px << PTLTH) | sb(Ap[((i + PTCOUNT) << PTLOG) | j].a, x);
				py = (py << PTLTH) | sb(Ap[(i << PTLOG) | j].a, y);
			}
			TBox[i][v] = px;
			TBox[i][v] = TBox[i][v] << (PTCOUNT * PTLTH);
			TBox[i][v] |= py;
		}
	}
}


void gen_sigma() {
	int k, i, j, x, y, r;
	
	for (k = 0; k < 2; k++) {
		int wtp, wt, wtr;
		wtp = k * PTCOUNT;
		wt = k * (PTCOUNT - 2);
		wtr = k * ROUND;
		
		for (i = 1; i < PTCOUNT - 1; i++) {
			for (j = 0; j < PTCOUNT; j++) {
				for (x = 0; x < STCOUNT; x++) {
					uint_8 xr, yr, r;
					if (i == PTCOUNT - 2) {
						xr = sb(Ap[cb3(i+1+wtp, j)].b, x);
					} else {
						xr = sb(A[cb3(i+wt, j)].b, x);
					}
					for (y = 0; y < STCOUNT; y++) {
						yr = sb(Ap[cb3(i+wtp, j)].b, y);
						r = sb(A[cb3(i-1+wt, j)].a, (xr ^ yr));
						if (y & 1) {
							SIGMA[cb3(i-1+wt, j)][cb4(x, y)>>1] |= (r<<PTLTH);
						}
						else {
							SIGMA[cb3(i-1+wt, j)][cb4(x, y)>>1] = r;
						}			
					}
				}
			}
		}

		for (r = 0; r < ROUND; r++) {
			for (j = 0; j < PTCOUNT; j++) {
				for (x = 0; x < STCOUNT; x++) {
					uint_8 xr, yr, mr;
					xr = sb(A[cb3(0+wt, j)].b, x);
					for (y = 0; y < STCOUNT; y++) {
						yr = sb(Ap[cb3(0+wtp, j)].b, y);
						mr = sb(As[cb3(r+wtr, j)].a, (xr ^ yr));
						if (y & 1) {
							SIGMAr[cb3(r+wtr, j)][cb4(x, y)>>1] |= (mr<<PTLTH);
						}
						else {
							SIGMAr[cb3(r+wtr, j)][cb4(x, y)>>1] = mr;
						}
					}
				}
			}
		}
	}
}


void print_enc() {
	int i, j;
	FILE *fp;

	fp = fopen("enc.h", "w");
	// head 
	fprintf(fp, "#include \"wb_lai_massey.h\"\n\n");

	// TBox
	fprintf(fp, "uint_64 TBox[PTCOUNT][STCOUNT * STCOUNT] = {\n");
	for (i = 0; i < PTCOUNT; i++) {
		fprintf(fp, "{ ");
		for (j = 0; j < (STCOUNT * STCOUNT); j++) {
			fprintf(fp, "0x%llx", TBox[i][j]);
			if (j == STCOUNT * STCOUNT - 1) {
				fprintf(fp, " }");
				if (i < PTCOUNT - 1) {
					fprintf(fp, ",\n");
				}
			} else {
				fprintf(fp, ", ");
			}
		}
	}
	fprintf(fp, "};\n");

	// SIGMA
	fprintf(fp, "uint_8 SIGMA[2 * (PTCOUNT - 2) * PTCOUNT][STCOUNT * STCOUNT / 2] = {\n");
	for (i = 0; i < (2 * (PTCOUNT - 2) * PTCOUNT); i++) {
		fprintf(fp, "{ ");
		for (j = 0; j < (STCOUNT * STCOUNT / 2); j++) {
			fprintf(fp, "0x%x", SIGMA[i][j]);
			if (j == STCOUNT * STCOUNT / 2 - 1) {
				fprintf(fp, " }");
				if (i < 2 * (PTCOUNT - 2) * PTCOUNT - 1) {
					fprintf(fp, ",\n");
				}
			} else {
				fprintf(fp, ", ");
			}
		}
	}
	fprintf(fp, "};\n");

	// SIGMAr
	fprintf(fp, "uint_8 SIGMAr[2 * ROUND * PTCOUNT][STCOUNT * STCOUNT / 2] = {\n");
	for (i = 0; i < (2 * ROUND * PTCOUNT); i++) {
		fprintf(fp, "{ ");
		for (j = 0; j < (STCOUNT * STCOUNT / 2); j++) {
			fprintf(fp, "0x%x", SIGMAr[i][j] & 0xff);
			if (j == STCOUNT * STCOUNT / 2 - 1) {
				fprintf(fp, " }");
				if (i < 2 * ROUND * PTCOUNT - 1) {
					fprintf(fp, ",\n");
				}
			} else {
				fprintf(fp, ", ");
			}
		}
	}
	fprintf(fp, "};\n");
	fclose(fp);
}


void print_dec() {
	int i, j;
	FILE *fp;
	fp = fopen("dec.h", "w");

	// Head
	fprintf(fp, "#include \"wb_lai_massey.h\"\n\n");
	// Hi
	fprintf(fp, "uint_64 Hi[PTCOUNT] = { ");
	for (i = 0; i < PTCOUNT; i++) {
		fprintf(fp, "0x%llx", H[i].b);
		if (i == PTCOUNT - 1) {
			fprintf(fp, " };\n");
		} else {
			fprintf(fp, ", ");
		}
	}

	// Gi
	fprintf(fp, "uint_64 Gi[PTCOUNT] = { ");
	for (i = 0; i < PTCOUNT; i++) {
		fprintf(fp, "0x%llx", G[i].b);
		if (i == PTCOUNT - 1) {
			fprintf(fp, " };\n");
		} else {
			fprintf(fp, ", ");
		}
	}

	// Asi
	fprintf(fp, "uint_64 Asi[2 * ROUND * PTCOUNT] = { ");
	for (i = 0; i < 2 * ROUND; i++) {
		for (j = 0; j < PTCOUNT; j++) {
			fprintf(fp, "0x%llx", As[(i << PTLOG) | j].b);
			if ((i == 2 * ROUND - 1) && (j == PTCOUNT - 1)) {
				fprintf(fp, " };\n");
			} else {
				fprintf(fp, ", ");
			}
		}
	}

	// Li
	fprintf(fp, "uint_32 Li[32] = { ");
	for (i = 0; i < 32; i++) {
		fprintf(fp, "0x%x", Li.r[i]);
		if (i < 31) {
			fprintf(fp, ", ");
		}
	}
	fprintf(fp, " };\n");

	// THETA
	fprintf(fp, "uint_64 THETA[PTCOUNT] = { ");
	for (i = 0; i < PTCOUNT; i++) {
		fprintf(fp, "0x%llx", THETA[i].a);
		if (i == PTCOUNT - 1) {
			fprintf(fp, " };\n");
		} else {
			fprintf(fp, ", ");
		}
	}

	// DELTA
	fprintf(fp, "uint_32 DELTA[32] = { ");
	for (int i = 0; i < 32; i++) {
		fprintf(fp, "0x%x", DELTA.r[i]);
		if (i < 31) {
			fprintf(fp, ", ");
		}
	}
	fprintf(fp, " };\n");	

	fclose(fp);
}


int main() {
	gen_tables();
	gen_tbox();
	gen_sigma();
	print_enc();
	print_dec();
	return 0;
}
