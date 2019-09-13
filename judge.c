#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "assist.h"
#include "nn.h"

int hidden_node, input_node, output_node, total_node, train_count, test_count, input_begin, input_end, hidden_begin, hidden_end, output_begin, output_end, dataset_type, dataset_input_size, dataset_output_size;
int sum[100];
double loss;
int incorrectnum;
double sigmoid(double x) {return 1.0 / (1.0 + exp(-x));}
double dsigmoid(double x) {return sigmoid(x) * (1.0 - sigmoid(x));}
double relu(double x) {return x > 0 ? x : 0.01 * x;}
double drelu(double x) {return x > 0 ? 1 : 0.01;}
double Sin(double x) {return sin(x);}
double dSin(double x) {return cos(x);}
int edge;
double weights[TOTAL_MAX_EDGE];
int head[TOTAL_MAX_EDGE], tail[TOTAL_MAX_EDGE], succ[TOTAL_MAX_EDGE], prev[TOTAL_MAX_EDGE], first[TOTAL_MAX];
double in[TOTAL_MAX], out[TOTAL_MAX];

void calc() {
	input_node = dataset_input_size + 1;
	output_node = dataset_output_size;
	hidden_node = sum[num];
	total_node = hidden_node + input_node + output_node;
	input_begin = 1, input_end = input_node, hidden_begin = input_node + 1, hidden_end = total_node - output_node, output_begin = total_node - output_node + 1, output_end = total_node;
} 

double* arrmax(double* begin, double* end) {
	double *ptr = begin, *ret = begin;
	while (ptr != end) {
		ptr++;
		if (*ptr > *ret) ret = ptr;
	}
	return ret;
}
void forward() {
	for (int i = hidden_begin;i <= output_end;i++) in[i] = out[i] = 0.0;
	for (int i = input_begin;i <= hidden_end;i++) {
		if (i >= hidden_begin && i <= hidden_end) out[i] = sigmoid(in[i]);
		for (int e = first[i];e;e = succ[e]) 
			in[tail[e]] += out[i] * weights[e];
	}
	double sigma = 0.0, D = *arrmax(in + output_begin, in + output_end + 1);
	for (int i = output_begin;i <= output_end;i++) sigma += exp(in[i] - D);
	for (int i = output_begin;i <= output_end;i++) out[i] = exp(in[i] - D) / sigma;
}
void addedge(int u, int v, double weight) {
	edge++;
	weights[edge] = weight;
	head[edge] = u;
	tail[edge] = v;
	succ[edge] = first[u];
	if (succ[edge]) prev[succ[edge]] = edge;
	first[u] = edge;
}

void nn_init() {
	FILE* f = fopen("weights", "rb");
	fread(first, sizeof(int), TOTAL_MAX, f);
	fread(succ, sizeof(int), TOTAL_MAX_EDGE, f);
	fread(prev, sizeof(int), TOTAL_MAX_EDGE, f);
	fread(tail, sizeof(int), TOTAL_MAX_EDGE, f);
	fread(weights, sizeof(double), TOTAL_MAX_EDGE, f);
	fclose(f);

	edge = 0;
	dataset_input_size = SIZE;
	dataset_output_size = 3;

	sum[0] = 0;
	for (int i = 1;i <= num;i++) sum[i] = sum[i - 1] + layer_nodes[i];

	calc();
}

int nn_predict(int* arr) {
	for (int i = 1;i < input_end;i++) 
		out[i] = arr[i];
	out[input_end] = 1.0;
				
	forward();

	double *p = arrmax(out + output_begin, out + output_end + 1);
	return (p - (out + output_begin)) + 1;
}
