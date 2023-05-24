#include "statistic.h"

static double arraysum(double numbers[], size_t len) {
	double res = 0;
	for(size_t i = 0; i < len; i++)
		res += numbers[i];	
	return res;
}

static void sqarray(double numbers[], size_t len) {
	for(size_t i = 0; i < len; i++) {
		numbers[i] *= numbers[i];
	}
}

static void subtoarray(double numbers[], size_t len, double num) {
	for(size_t i = 0; i < len; i++) {
		numbers[i] -= num;
	}
}

double avg(double numbers[], size_t len) {
	if (!(len)) return 0;
	return arraysum(numbers, len)/len;
}

static double* deepcopy(double numbers[], size_t len) {
	double* copy = malloc(len * sizeof(double));
	if (copy == NULL) exit(EXIT_FAILURE);

	for (size_t i = 0; i < len; i++) {
		copy[i] = numbers[i];
	}
	
	return copy;
}

double std(double numbers[], size_t len) {
	if (len < 2) return 0;
	
	double* temp = deepcopy(numbers, len);
	
	double average = avg(numbers, len);
	subtoarray(temp, len, average);
	sqarray(temp, len);
	double res = sqrt(arraysum(temp, len)/len);	
	
	free(temp);
	return res;	
}
