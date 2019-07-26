#include <stdio.h>	
#include "assist.h"
#include <sys/socket.h> 	
#include <sys/un.h> 	
#include <stddef.h> 	
#include <time.h>	
#include <string.h>	
#include <unistd.h>	
int get_voltage() {	
	FILE* f = fopen("/sys/devices/3160000.i2c/i2c-0/0-0040/iio_device/in_voltage0_input", "r");	
	int ret;	
	fscanf(f, "%d\n", &ret);	
	fclose(f);	
	return ret;	
}	
char msg[MAXBUF];	
int main() {	
	while (1) {	
		sprintf(msg, "%d\n", get_voltage());	
		submit("assist.sock", msg);
		sleep(10);
	}	
	return 0;	
}
