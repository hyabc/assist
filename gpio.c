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
	char path[100];
	sprintf(path, "/sys/class/gpio/gpio%d/value", port);
	FILE* f = fopen(path, "w");
	fprintf(f, "%d\n", value);
	fclose(f);
}
int gpio_get_value(int port) {
	char path[100];
	sprintf(path, "/sys/class/gpio/gpio%d/value", port);
	FILE* f = fopen(path, "r");
	int ret;
	fscanf(f, "%d\n", &ret);
	fclose(f);
	return ret;
}
void ultrasonic(int port_trig, int port_echo) {
	char path[100];
	FILE* f;
	sprintf(path, "/sys/class/gpio/gpio%d/direction", port_trig);
	f = fopen(path, "w");
	fputc('1', f);
	fclose(f);
	usleep(20);
	f = fopen(path, "w");
	fputc('0', f);
	fclose(f);

	sprintf(path, "/sys/class/gpio/gpio%d/value", port_echo);
	int ret, cnt = 0;
	clock_t start = clock(), end;
	while (true) {
		f = fopen(path, "r");
		ret = fgetc(f) - '0';
		fclose(f);
		if (ret == 0) break;
	}
	end = clock();
	printf("%f\n", (double)(end - start) / CLOCKS_PER_SEC * 17150);
}
int main() {
	gpio_export(466);
	gpio_export(397);
	sleep(1);
	gpio_set_direction(466, "out");
	gpio_set_direction(397, "in");
	ultrasonic(466, 397);
	gpio_unexport(466);
	gpio_unexport(397);
	return 0;
}
