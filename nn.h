#ifndef __NN_H__
#define __NN_H__

#define TOTAL_MAX 1000
#define TOTAL_MAX_EDGE 10000
#define num 2
extern const int layer_nodes[];

void nn_init();
int nn_predict(int*);

#endif

