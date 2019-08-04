#ifndef __NN_H__
#define __NN_H__

#define TOTAL_MAX 1000
#define TOTAL_MAX_EDGE 10000
const int num = 2;
const int layer_nodes[20] = {0, 30, 20};

void nn_init();
int nn_predict(int*);

#endif

