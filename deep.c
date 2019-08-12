#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "assist.h"
#include "nn.h"
const double learning_rate = 1e-4;
const double alpha = 0.001, beta1 = 0.9, beta2 = 0.999, eps = 1e-6;
#define epoch_max 100000
const int epoches = 2000;
int hidden_node, input_node, output_node, total_node, train_count, test_count, input_begin, input_end, hidden_begin, hidden_end, output_begin, output_end, dataset_type, dataset_input_size, dataset_output_size;
int sum[20];
double loss;
double epoch_loss[epoch_max], epoch_accuracy[epoch_max];
int pointer;
int incorrectnum;
int timestamp;
double sigmoid(double x) {return 1.0 / (1.0 + exp(-x));}
double dsigmoid(double x) {return sigmoid(x) * (1.0 - sigmoid(x));}
double relu(double x) {return x > 0 ? x : 0.01 * x;}
double drelu(double x) {return x > 0 ? 1 : 0.01;}
double Sin(double x) {return sin(x);}
double dSin(double x) {return cos(x);}
int training_set_label[70010];
double training_set_image[70010][100];
int test_set_label[10010];
double test_set_image[10010][100];
int edge;
double weights[TOTAL_MAX_EDGE], edge_gradient_sum[TOTAL_MAX_EDGE];
int head[TOTAL_MAX_EDGE], tail[TOTAL_MAX_EDGE], succ[TOTAL_MAX_EDGE], prev[TOTAL_MAX_EDGE], first[TOTAL_MAX];
double in[TOTAL_MAX], out[TOTAL_MAX], din[TOTAL_MAX], dout[TOTAL_MAX];

void calc() {
	input_node = dataset_input_size + 1;
	output_node = dataset_output_size;
	hidden_node = sum[num];
	total_node = hidden_node + input_node + output_node;
	input_begin = 1, input_end = input_node, hidden_begin = input_node + 1, hidden_end = total_node - output_node, output_begin = total_node - output_node + 1, output_end = total_node;
} 
double power(double base, int pt) {
	double a = 1.0;
	while (pt) {
		if (pt & 1) a = a * base;
		base = base * base;
		pt >>= 1;
	}
	return a;
}
void printtime() {
	time_t t = time(0);
	struct tm *tmp = localtime(&t);
	printf("%02d:%02d:%02d", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}	
void init() {
	memset(first, 0, sizeof(first));
		
	dataset_input_size = SIZE;
	dataset_output_size = 3;
	train_count = 200, test_count = 101;

	FILE* train = fopen("train-data", "r");
	for (int i = 1;i <= train_count;i++) {
		for (int j = 1;j <= dataset_input_size;j++) {
			int t;fscanf(train, "%d", &t);
			training_set_image[i][j] = t;
		}
		fscanf(train, "%d", &training_set_label[i]);
		training_set_label[i]--;
	}
	fclose(train);
	FILE* test = fopen("test-data", "r");
	for (int i = 1;i <= test_count;i++) {
		for (int j = 1;j <= dataset_input_size;j++) {
			int t;fscanf(train, "%d", &t);
			test_set_image[i][j] = t;
		}
		fscanf(test, "%d", &test_set_label[i]);
		test_set_label[i]--;
	}
	fclose(test);
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
		if (i >= hidden_begin && i <= hidden_end) out[i] = relu(in[i]);
		for (int e = first[i];e;e = succ[e]) 
			in[tail[e]] += out[i] * weights[e];
	}
	double sigma = 0.0, D = *arrmax(in + output_begin, in + output_end + 1);
	for (int i = output_begin;i <= output_end;i++) sigma += exp(in[i] - D);
	for (int i = output_begin;i <= output_end;i++) out[i] = exp(in[i] - D) / sigma;
}
void backward() {
	for (int i = hidden_end;i >= hidden_begin;i--) {
		dout[i] = 0.0;
		for (int e = first[i];e;e = succ[e]) {
			dout[i] += din[tail[e]] * weights[e];
			double gradient = din[tail[e]] * out[i];
			edge_gradient_sum[e] += gradient;
		}
		din[i] = dout[i] * drelu(in[i]);
	}
}
void train_thread(int c) {
	for (int i = 1;i < input_end;i++) out[i] = training_set_image[c][i];
	out[input_end] = 1.0;

	forward();
				
	for (int i = output_begin;i <= output_end;i++) 
		//loss += 1.0 / (double)(output_node) *  (out[mem][i] - (training_set_label.b[c] == (i - output_begin) ? 1 : 0)) * (out[mem][i] - (training_set_label.b[c] == (i - output_begin) ? 1 : 0));
		loss += -log(out[i] + eps) * (training_set_label[c] == (i - output_begin) ? 1 : 0);

	//for (int i = output_begin;i <= output_end;i++) printf("%f ", out[mem][i]);printf("\n");
	/*for (int i = output_end;i >= output_begin;i--) {
		dout[mem][i] = 1.0 / (double)(output_node) * (2 * out[mem][i] - 2 * (training_set_label.b[c] == (i - output_begin) ? 1 : 0));
	}
	for (int i = output_end;i >= output_begin;i--) {
		din[mem][i] = 0.0;
		for (int j = output_end;j >= output_begin;j--) din[mem][i] += dout[mem][j] * ((i != j) ? (-out[mem][i] * out[mem][j]) : (out[mem][i] * (1.0 - out[mem][j])));
	}*/
	for (int i = output_end;i >= output_begin;i--) din[i] = out[i] - (training_set_label[c] == (i - output_begin) ? 1 : 0);

	backward();

}
void test_thread(int c) {
	for (int i = 1;i < input_end;i++) out[i] = test_set_image[c][i];
	out[input_end] = 1.0;
				
	forward();

	double *p = arrmax(out + output_begin, out + output_end + 1);
	if ((p - (out + output_begin)) != test_set_label[c]) incorrectnum++;
}
double test() {
	incorrectnum = 0;
	for (int c = 1;c <= test_count;c++) {
		test_thread(c);
	}
	return 100 * (double)(test_count - incorrectnum) / test_count;
}
void train() {
	for (int c = 1;c <= train_count;c++) {
		for (int j = 1;j <= edge;j++) edge_gradient_sum[j] = 0.0;
		loss = 0.0;
		train_thread(c);
		for (int j = 1;j <= edge;j++) weights[j] -= learning_rate * edge_gradient_sum[j];
		epoch_loss[pointer] += loss;
//		printf("\r");printtime();printf(" [%d-%d]   loss: %f            ", c, c + batch_size - 1, loss);
	}
}
double Random() {return ((double)(rand() % 1000 - 500.0) / 500.0);}
void addedge(int u, int v, double weight) {
	edge++;
	weights[edge] = weight;
	head[edge] = u;
	tail[edge] = v;
	succ[edge] = first[u];
	if (succ[edge]) prev[succ[edge]] = edge;
	first[u] = edge;
}
int main() {
	edge = 0;
	srand(time(0));
	init();

	/*printf("Enter hidden layer: ");
	scanf("%d", &num);
	for (int i = 1;i <= num;i++) scanf("%d", &layer[i]);*/
	sum[0] = 0;
	for (int i = 1;i <= num;i++) sum[i] = sum[i - 1] + layer_nodes[i];

	calc();
	

	for (int i = hidden_begin;i < hidden_begin + sum[1];i++)
		for (int j = 1;j < input_node;j++) addedge(j, i, Random() / sqrt(input_node - 1));
		
	for (int k = 2;k <= num;k++) 
		for (int i = hidden_begin + sum[k - 1];i < hidden_begin + sum[k];i++) 
			for (int j = hidden_begin + sum[k - 2];j < hidden_begin + sum[k - 1];j++) addedge(j, i, Random() / sqrt(layer_nodes[k - 1]));
		
	for (int i = output_begin;i <= output_end;i++) 
		for (int j = hidden_begin + sum[num - 1];j < hidden_begin + sum[num];j++) addedge(j, i, Random() / sqrt(layer_nodes[num]));


	for (int i = hidden_begin;i <= output_end;i++) addedge(input_end, i, 0);

	for (int epoch = 1;epoch <= epoches;epoch++) {
		pointer++;
		train();
		printtime();printf("  %d:   accuracy %f, loss %f \n", epoch, epoch_accuracy[pointer] = test(), epoch_loss[pointer]);
	}

//	for (int i = 1;i <= epoch_count;i++) printf(" %d/%d: accuracy %f                       \n", i, epoch_count, train());
	printf("Accuracy: %f\n", test());
	printf("END:");printtime();printf("\n");
	
/*	printf("x=1:1:%d;a=[", pointer);
	for (int i = 1;i < pointer;i++) printf("%f,", epoch_loss[i]);
	printf("%f];b=[", epoch_loss[pointer]);
	for (int i = 1;i < pointer;i++) printf("%f,", epoch_accuracy[i]);
	printf("%f];y=1:1:%d;c=[", epoch_accuracy[pointer], epoches);
	for (int i = 1;i < epoches;i++) printf("%d,", epoch_del[i]);
	printf("%d];d=[", epoch_del[epoches]);
	for (int i = 1;i < epoches;i++) printf("%d,", epoch_add[i]);
	printf("%d];\n", epoch_add[epoches]);
	printf("==============================================\n");*/

	FILE* f = fopen("weights", "wb");
	fwrite(first, sizeof(int), TOTAL_MAX, f);
	fwrite(succ, sizeof(int), TOTAL_MAX_EDGE, f);
	fwrite(prev, sizeof(int), TOTAL_MAX_EDGE, f);
	fwrite(tail, sizeof(int), TOTAL_MAX_EDGE, f);
	fwrite(weights, sizeof(double), TOTAL_MAX_EDGE, f);
	fclose(f);


	/*while (1) {
		for (int i = 1;i < input_end;i++) {
			int t;scanf("%d", &t);
			out[i] = t;
		}
		out[input_end] = 1.0;
					
		forward();

		double *p = arrmax(out + output_begin, out + output_end + 1);
		int ans = (p - (out + output_begin)) + 1;
		printf("%d\n", ans);
	}*/
	

	return 0;
}
