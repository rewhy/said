#pragma once

void split(uint_8 *text, uint_8 *x);
void join(uint_8 *result, uint_8 *x);
uint_8 *disassemble(uint_32 vec);
uint_32 assemble(uint_8 *a);
uint_8 sb(uint_64 sbox, uint_8 vec);
void lg(uint_8 *x, char c);
void or(uint_8 *x, uint_8 *rslt);
void ori(uint_8 *x, uint_8 *rslt);
int cb3(int x, int y);
int cb4(int x, int y);