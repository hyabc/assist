#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
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
	fscanf(f, "%d\n", &ret);
	fclose(f);
	return ret;
}
void ultrasonic(int port_trig, int port_echo) {
	gpio_set_value(port_trig, 0);
	usleep(100);
	gpio_set_value(port_trig, 1);
	usleep(20);
	gpio_set_value(port_trig, 0);
	int ret, state = 0;
	clock_t start;
	for (int i = 1;i <= 150;i++) {
		ret = gpio_get_value(port_echo);
		//printf("%d", ret);
		if (!state && ret) {state = 1;start = clock();}
		if (state && !ret) break;
	}
	printf("%f\n", (double)(clock() - start) / CLOCKS_PER_SEC * 17150);
}
int main() {
	gpio_export(466);
	gpio_export(397);
	sleep(1);
	while (1) {
		ultrasonic(466, 397);
		usleep(200000);
	}
	gpio_unexport(466);
	gpio_unexport(397);
	return 0;
}
