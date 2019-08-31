#include <stdio.h>
#include "assist.h"
FILE* x, *y;
int n, a;
int main() {
	y = fopen("value4", "w");

	x = fopen("value1", "r");
	fscanf(x, "%d", &n);
	for (int j = 1;j <= n;j++) {
		for (int i = 1;i <= SIZE;i++) {
			fscanf(x, "%d", &a);
			fprintf(y, "%d ", a);
		}
		fprintf(y, "%d\n", 1);
	}
	fclose(x);

	x = fopen("value2", "r");
	fscanf(x, "%d", &n);
	for (int j = 1;j <= n;j++) {
		for (int i = 1;i <= SIZE;i++) {
			fscanf(x, "%d", &a);
			fprintf(y, "%d ", a);
		}
		fprintf(y, "%d\n", 2);
	}
	fclose(x);

	x = fopen("value3", "r");
	fscanf(x, "%d", &n);
	for (int j = 1;j <= n;j++) {
		for (int i = 1;i <= SIZE;i++) {
			fscanf(x, "%d", &a);
			fprintf(y, "%d ", a);
		}
		fprintf(y, "%d\n", 3);
	}
	fclose(x);
	
	fclose(y);
	return 0;

}
