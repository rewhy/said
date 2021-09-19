// svmDetect.cpp

#include <stdio.h>
#include <stdlib.h>
#include <cutils/log.h>


#include "libsvm/svm.h"

#include "utils.h"
#include "log.h"
#include "extFeature.h"
#include "svmDetect.h"


#define MODEL_FILE_PATH		"/sdcard/andbModel.txt"
#define OUTPUT_FILE_PATH	"/sdcard/andbOutput.txt"


static int (*info)(const char *fmt,...) = &myLog;

static svm_model *s_model = NULL;
static svm_node  x_node[MAX_NR_ATTR];

static char line[MAX_LINE_LEN];

static int predict_probability = 0;


static void exit_input_error(int err_num) {
}

double predict( double *feature ) {
	double res = 0.0f;

	double predict_label;
	char	 *label;
	int i = 0, j = 0, max_nr_attr = MAX_NR_ATTR;
	int correct = 0, total = 0;

	int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

	double *prob_estimates = NULL;

	int svm_type = svm_get_svm_type(s_model);
	int nr_class = svm_get_nr_class(s_model);

	if( predict_probability ) {
		if(svm_type == EPSILON_SVR || svm_type == NU_SVR) {
			info("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(s_model));
		} else {
			int *labels=(int *) malloc(nr_class*sizeof(int));
			svm_get_labels(s_model, labels);
			prob_estimates = (double *) malloc(nr_class*sizeof(double));
			free(labels);
		}
	}

	i = 0;
	while(1) {
		
		if(feature[i] == (double)0xffffffff)
			break;
		x_node[i].index = i+1;
		x_node[i].value = feature[i];

		++i;
	}

	x_node[i].index = -1;
	
	if(predict_probability == 1 && (svm_type == C_SVC, svm_type == NU_SVC)) {
		predict_label = svm_predict_probability(s_model, x_node, prob_estimates);
		myLog("%g", predict_label);
	} else {
		predict_label = svm_predict(s_model, x_node);
		myLog("%.17g\n", predict_label);
	}

	if(prob_estimates) {
		free(prob_estimates);
		prob_estimates = NULL;
	}

	return predict_label;
}

void stateInfer() {
	double res = 0.01;
	ullong ts = getCurrentTime(), last_ts = 0;
	while(true) {
		double *feature = getWinFeature();
		res = predict(feature);
		last_ts = getCurrentTime();
		break;
	}
}

bool initSVMModel() {

	FILE *output = fopen(OUTPUT_FILE_PATH, "w");
	if(output == NULL)
	{
		ALOGE("Can't open output file %s\n", OUTPUT_FILE_PATH);
		return false;
	}

	if((s_model=svm_load_model(MODEL_FILE_PATH))==0) {
		ALOGE("Can't open model file %s", MODEL_FILE_PATH);
		return false;
	}

	return true;
}


