// extFeature.cpp

#include <stdio.h>
#include <stdlib.h>


#include "utils.h"
#include "extFeature.h"

#define MAX_NODE_NUM				1024
/* The detection window */
#define DEC_WIN_PERIOD			1000*1000*3
/* The scale step */
#define DEC_SCALE_STEP			1000*1000
#define MAX_MRIX_SIZE				128*128

#define F_MAX_IDX				0
#define F_MIN_IDX				1
#define F_RNG_IDX				2
#define F_AVG_IDX				3
#define F_STD_IDX				4



static struct ivInfoNode	iv_info_ring[MAX_NODE_NUM];
static struct gyInfoNode	gy_info_ring[MAX_NODE_NUM];
static uint iv_info_idx = 0;
static uint gy_info_idx = 0;

static double sensor_data_matrix[MAX_NODE_NUM][6];
static int sensor_matrix_row = 0;

static inline double getS(double v) {
	return v;
}

static double getMax(double *values, int size) {
	double ret = values[0];
	for (int i = 1; i < size; i++) {
		if(ret < values[i])
			ret = values[i];
	}
	return ret;
}

static double getMin(double *values, int size) {
	double ret = values[0];
	for (int i = 1; i < size; i++) {
		if(ret > values[i])
			ret = values[i];
	}
	return ret;
}

static double getSum(double *values, int size) {
	double ret = 0;
	for (int i = 0; i < size; i++) {
			ret += values[i];
	}
	return ret;
}

static double getMean(double *values, int size) {
	double ret = 0;
	for (int i = 0; i < size; i++) {
			ret += values[i];
	}
	ret = ret / (double)size;
	return ret;
}

static double getSQ(double *values, int size) { // Sum of squares
	double ret = 0.0f;
	for (int i = 0; i < size; i++) {
			ret += values[i] * values[i];
	}
	return ret;
}

static double getVariance(double *values, int size) {
	double sq = getSQ(values, size);
	double avg = getMean(values, size);
	double ret = (sq/(double)size) - (avg * avg);
	return ret;
}

static double getSTD(double *values, int size) {
	double ret = 0.0f;
	double avg = getMean(values, size);
	for(int i = 0; i < size; i++)
		ret += getS(values[i]-avg);
	ret /= (double)size;
	return ret;
}

static double getRange(double *values, int size) {
	return getMax(values, size) - getMin(values, size);
}

void addGyNode(ullong ts, double *data) {
	struct gyInfoNode *node = &gy_info_ring[gy_info_idx];
	node->ts = ts;
	memcpy((void*)node->data, (void*)data, 6 * sizeof(double));
	gy_info_idx = (gy_info_idx+1) % MAX_NODE_NUM;
}

void addIvNode(ullong ts, uint type, double data) {
	struct ivInfoNode *node = &iv_info_ring[iv_info_idx];
	node->ts = ts;
	node->type = type;
	node->value = data;
	iv_info_idx = (iv_info_idx+1) % MAX_NODE_NUM;
}


static void DESF(double *data, double alph, int col, int row) {
	int c = 0, r = 0;
	double xi_1  = 0.0f;

	for (c = 0; c < col; c++){
		xi_1 = data[c*col]; // data[c][0]
		for (r = 1; r < row; r++){
			if (data[r*col+c] >= xi_1){ // data[r][c]
				// data[r][c] = (1 - alph) * xi_1 + alph * data[r][c];
				data[r*col+c] = (1 - alph) * xi_1 + alph * data[r*col+c];
			}
			else {
				// data[r][c] = alph * xi_1 + (1 - alph) * data[r][c];
				data[r*col+c] = alph * xi_1 + (1 - alph) * data[r*col+c];
			}
			// xi_1 = data[r][c];
			xi_1 = data[r*col+c];
		}
	}
}


#define MAX_DETECT_ITEM		128
#define MAX_FEATURE_NUM		128
static double features[MAX_FEATURE_NUM];
static int f_num = 0;

static int addFeatures(double *data, int col, int row) {

	double tmp[MAX_DETECT_ITEM] = {0.0f};
	
	int r = 0, c = 0;
	for(c = 0; c < col; c++) {
		for(r = 0; r < row; r++) {
			// tmp[r] = data[r][c];
			tmp[r] = data[r*col+c];
			if(r >= MAX_DETECT_ITEM)
				break;
		}

		features[f_num++] = getMax(tmp, r);
		features[f_num++] = getMin(tmp, r);
		features[f_num++] = getRange(tmp, r);
		features[f_num++] = getMean(tmp, r);
		features[f_num++] = getSTD(tmp, r);
	}
	return c;
}

double *getWinFeature() {
	int i = gy_info_idx, j = 0;
	struct gyInfoNode *gy_node = NULL;
	ullong cur_ts = getCurrentTime();
	while(true) {
		gy_node = &gy_info_ring[(i+MAX_NODE_NUM-1) % MAX_NODE_NUM];
		if(gy_node->ts == 0 || (gy_node->ts + DEC_WIN_PERIOD) < cur_ts)
			break;
		sensor_data_matrix[j][0] = gy_node->data[0];
		sensor_data_matrix[j][1] = gy_node->data[1];
		sensor_data_matrix[j][2] = gy_node->data[2];
		sensor_data_matrix[j][3] = gy_node->data[3];
		sensor_data_matrix[j][4] = gy_node->data[4];
		sensor_data_matrix[j][5] = gy_node->data[5];
		j++;
	}

	// 6 columans and #j# rows 
	DESF((double *)sensor_data_matrix, 0.6, 6, j);

	f_num = 0;
	addFeatures((double *)sensor_data_matrix, 6, j);


	// TODO: More features can be added

	features[f_num] = (double)0xffffffff;
	return features;
}



#if 0
double* get1_value(double **data, int col, int row) {

	int c = 0, r = 0, i = 0;
	double s1 = 0.0f, st = 0.0f, a = 0.76f;
	double *pre_values = (double*)malloc(row * sizeof(double));
	double *S_1 = (double*)malloc(row * (col + 1) * sizeof(double));

	for (r = 0; r < row; r++) {
		for (c = 0; c < col; c++) {
			if (c < 4) {
				s1 += data[r][c];
				if (c < 20)
					st=s1/3.0f;
				else
					st=data[r][0];
			}
		}

		S_1[i++] = st;

		for (c = 0; c < col ; c++) {
			S_1[i++] = a * data[r][c] + (1 - a) * S_1[c];
		}
		pre_values[r]=S_1[i-1];
	}

	free(S_1);

	return pre_values;
}


static double** get2_valueQue(double *qWin, int t, int size) {

	QueueNode myQue = qWin.front;

	double data_all = (double *)malloc(q

			int queLen = qWin.queueSize();
			double[][] data_all = new double[5][queLen];
			int z11 = 0;
			double[] temp;
			while(myQue != null){
			//                                //             temp = qWin.dequeue();
			temp = myQue.data;
			myQue = myQue.next;
			data_all[0][z11] = temp[0];
			data_all[1][z11] = temp[1];
			data_all[2][z11] = temp[2];
			data_all[3][z11] = temp[3];
			data_all[4][z11] = temp[4];
			z11++;
			}
			double[][] pre_values = data_all;
			for (int z = 0; z < data_all.length -1; z++){
			double[] data = data_all[z];

			ArrayList<Double> S_1 = new ArrayList<Double>();
			ArrayList<Double> S2_1_new = new ArrayList<Double>();
			ArrayList<Double> S2_2_new = new ArrayList<Double>();


			double s1 = 0.0;
			double st = 0.0;
			double at = 0.0;
			double bt = 0.0;
			double xt = 0.0;

			for (int j = 1; j < data.length; j++) {

				if (j < 4) {
					s1 += data[j - 1];
					if (data.length < 20)
						st = s1 / 3;
					else
						st = data[0];
				}
			}
			S_1.add(st);
			for (int k = 0; k < data.length ; k++) {
				if(k==0)
					S2_1_new.add(a * data[k] + (1 - a) * S_1.get(k));
				else
					S2_1_new.add(a*data[k]+(1-a)*S2_1_new.get(k-1));
			}
			for (int l = 0; l <data.length ; l++) {
				if(l==0){
					double tmp = a * S2_1_new.get(l) + (1 - a) * S_1.get(l);
					S2_2_new.add(tmp);
					pre_values[z][l] = tmp;
				}
				else{
					double tmp = a*S2_1_new.get(l)+(1-a)*S2_2_new.get(l-1);
					S2_2_new.add(a*S2_1_new.get(l)+(1-a)*S2_2_new.get(l-1));
					pre_values[z][l] = tmp;

				}
			}
			}
			return pre_values;
}
#endif
