// extFeature.h
//
#ifndef __EXTRACT_FEATURE_H
#define __EXTRACT_FEATURE_H

#include "utils.h"



// Onboard information node
struct gyInfoNode {
	ullong ts;
	double data[6];
};

// in-vehicle info node
struct ivInfoNode {
	ullong ts;
	uint   type;
	double value;
};

// void DESF(double **data, double alph, int col, int row);
// double* get1_value(double **data, int col, int row);
//
double *getWinFeature();
#endif // __EXTRACT_FEATURE_H
