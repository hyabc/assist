#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#define AVERAGE_TIME 10
void gpio_export(int port) {
	FILE* f = fopen("/sys/class/gpio/export", "w");
	fprintf(f, "%d\n", port);
	fclose(f);
}
void gpio_unexport(int port) {
	FILE* f = fopen("/sys/class/gpio/unexport", "w");
	fprintf(f, "%d\n", port);
	fclose(f);
}
void gpio_set_direction(int port, const char* str) {
	char path[100];
	sprintf(path, "/sys/class/gpio/gpio%d/direction", port);
	FILE* f = fopen(path, "w");
	fprintf(f, "%s\n", str);
	fclose(f);
}
void gpio_set_value(int port, int value) {
	gpio_set_direction(port, "out");
	char path[100];
	sprintf(path, "/sys/class/gpio/gpio%d/value", port);
	FILE* f = fopen(path, "w");
	fprintf(f, "%d\n", value);
	fclose(f);
}
int gpio_get_value(int port) {
	gpio_set_direction(port, "in");
	char path[100];
	sprintf(path, "/sys/class/gpio/gpio%d/value", port);
	FILE* f = fopen(path, "r");
	int ret;
	fscanf(f, "%d", &ret);
	fclose(f);
	return ret;
}
struct ultrasonic_sensor {
	int port_trig, port_echo;
	ultrasonic_sensor(int port_trig, int port_echo): port_trig(port_trig), port_echo(port_echo) {
		gpio_export(port_trig);
		gpio_export(port_echo);
		sleep(1);
	}
	~ultrasonic_sensor() {
		gpio_unexport(port_trig);
		gpio_unexport(port_echo);
	}
	double get_distance() {
		gpio_set_value(port_trig, 0);
		usleep(100);
		gpio_set_value(port_trig, 1);
		usleep(20);
		gpio_set_value(port_trig, 0);
		int ret, state = 0;
		clock_t start;
		for (int i = 1;i <= 200;i++) {
			ret = gpio_get_value(port_echo);
	//		printf("%d", ret);
			if (!state) {start = clock();if (ret) state = 1;}
			if (state && !ret) break;
		}
		return (double)(clock() - start) / CLOCKS_PER_SEC * 17150;
	}
	double get_average_distance() {
		double sum = 0;
		for (int i = 1;i <= AVERAGE_TIME;i++) {
			sum += get_distance();
			usleep(100);
		}
		return sum / AVERAGE_TIME;
	}
};
char response[500];
int main() {
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "assist.sock");

	ultrasonic_sensor s1(466, 397), s2(392, 296), s3(481, 254);
	sleep(1);
	while (1) {
		sprintf(response, "U%lf %lf %lf", s1.get_average_distance(), s2.get_average_distance(), s3.get_average_distance());
		printf("%s\n", response);
/*
		int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
		send(sockfd, response, strlen(response) + 1, 0);
		close(sockfd);
*/
		usleep(200);
	}
	return 0;
}
