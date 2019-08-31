#include <stdlib.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <sstream>
#include <termios.h> 
#include <string.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <stdio.h>
extern "C" {
#include "assist.h"
}

#define OFFSET 0

char buf[MAXBUF], msg[MAXBUF], st[MAXBUF];
int servofd, laserfd;
int dist[SIZE], base[SIZE];

unsigned char data[9];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int distance, strength;

int serialport_init(const char* serialport) {
	struct termios tty;
	int fd = open(serialport, O_RDWR | O_NOCTTY);

	tcgetattr(fd, &tty);
	speed_t brate = B115200;
	cfsetispeed(&tty, brate);
	cfsetospeed(&tty, brate);

	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CREAD | CLOCAL;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tty.c_oflag &= ~OPOST;

	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 0;

	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &tty);

	return fd;
}

void serialport_write(int fd, int x) {
	sprintf(buf, "%d\n", x);
	write(fd, buf, strlen(buf));
	tcflush(fd, TCIOFLUSH);
}

char serialport_getchar() {
	char value;
	while (read(laserfd, &value, 1) != 1) ;
	return value;
}

void* laser_receive(void* arg) {
	int pos, cur_distance, cur_strength;
	while (true) {
		pos = 0;
		while ((buf[pos] = serialport_getchar()) != '\n') pos++;

		std::string str(buf, pos);
		std::stringstream ss(str);

		ss >> cur_distance >> cur_strength;

		if (cur_distance >= 30 && cur_distance <= 500) {
			pthread_mutex_lock(&mutex);
			distance = cur_distance;
			strength = cur_strength;
			pthread_mutex_unlock(&mutex);
		}
	}
}

void measure() {

	for (int angle = MIN_ANGLE;angle <= MAX_ANGLE;angle += DELTA_ANGLE) {
		serialport_write(servofd, angle + OFFSET);
		usleep(20000);

		pthread_mutex_lock(&mutex);
		dist[(angle - MIN_ANGLE) / DELTA_ANGLE] = distance;
		pthread_mutex_unlock(&mutex);

	}

	for (int angle = MAX_ANGLE;angle >= MIN_ANGLE;angle -= DELTA_ANGLE) {
		serialport_write(servofd, angle + OFFSET);
		usleep(20000);
	}
}

int main() {
	servofd = serialport_init("/dev/ttyACM2");
	laserfd = serialport_init("/dev/ttyACM0");

	pthread_t laser_thread;
	pthread_create(&laser_thread, NULL, laser_receive, NULL);

	sleep(1);

	memset(base, 0, sizeof(base));

	serialport_write(servofd, MIN_ANGLE + OFFSET);
	measure();

	for (int iter = 1;iter <= LASER_CALIBRATION_NUM;iter++) {
		measure();
		for (int i = 0;i < SIZE;i++) printf("%d ", dist[i]);
		printf("\n");
		for (int i = 0;i < SIZE;i++) 
			base[i] += dist[i];
	}

	for (int i = 0;i < SIZE;i++) base[i] /= LASER_CALIBRATION_NUM;
	printf("========================CALIBRATION============================\n");
	for (int i = 0;i < SIZE;i++) printf("%d ", base[i]);
	printf("\n===============================================================\n");

	FILE* fd = fopen("value3", "a");
	for (int iter = 1;;iter++) {
	    gets(st);
		if (strcmp(st, "end") == 0) break;

		measure();

		std::stringstream ss;
		for (int i = 0;i <= (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE;i++)
			ss << dist[i] - base[i] << " ";
		printf("%s\n\n", ss.str().c_str());
		fprintf(fd, "%s\n", ss.str().c_str());

	}

	close(servofd);
	close(laserfd);
	fclose(fd);
	return 0;
}

