#include <stdio.h>
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
char msg[100];
int main() {
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "assist.sock");
	while (1) {
		sprintf(msg, "%d\n", get_voltage());
		int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
		send(sockfd, msg, strlen(msg), 0);
		close(sockfd);
	}
	return 0;
}
