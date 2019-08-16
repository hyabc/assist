#include <stdlib.h>
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

#define header 0x59

char buf[MAXBUF], msg[MAXBUF];
int servofd, laserfd;
int dist[SIZE], base[SIZE], cnt[SIZE], tot;

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

	tty.c_cc[VMIN]  = 1;
	tty.c_cc[VTIME] = 0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tty);
	//tcsetattr(fd, TCSAFLUSH, &tty);

	return fd;
}

void serialport_write(int fd, int x) {
	sprintf(buf, "%d\n", x);
	write(fd, buf, strlen(buf));
	tcflush(fd, TCIOFLUSH);
}

unsigned char get_laser() {
	unsigned char value;
	while (read(laserfd, &value, 1) != 1) ;
	return value;
}

void* laser_receive(void* arg) {
	while (true) {
		if (get_laser() == header) {
			data[0] = header;
			if (get_laser() == header) {
				data[1] = header;

				for (int i = 2;i < 9;i++) 
					data[i] = get_laser();

				unsigned char checksum = 0;
				for (int i = 0;i < 8;i++) 
					checksum += data[i];

				if (data[8] == checksum) {
					pthread_mutex_lock(&mutex);
					distance = (int)(data[2]) + (int)(data[3]) * 256;
					strength = (int)(data[4]) + (int)(data[5]) * 256;
					printf("%d, %d\n", distance, strength);
				}
			}
		}
	}
}

/*void measure() {

	for (int angle = MIN_ANGLE;angle <= MAX_ANGLE;angle += DELTA_ANGLE) {
		serialport_write(serialfd, angle + OFFSET);
		usleep(20000);


//		dist[(angle - MIN_ANGLE) / DELTA_ANGLE] = measurementdata.RangeMilliMeter;

	}

	for (int angle = MAX_ANGLE;angle >= MIN_ANGLE;angle -= DELTA_ANGLE) {
		serialport_write(serialfd, angle + OFFSET);
		usleep(200);

	}
	serialport_write(serialfd, MIN_ANGLE + OFFSET);
	usleep(200000);
}*/

int main() {
	servofd = serialport_init("/dev/ttyACM0");
	laserfd = serialport_init("/dev/ttyS0");

	pthread_t laser_thread;
	pthread_create(&laser_thread, NULL, laser_receive, NULL);
	pause();
/*


	memset(base, 0, sizeof(base));
	memset(cnt, 0, sizeof(cnt));
	tot = 0;

	serialport_write(serialfd, MIN_ANGLE + OFFSET);
	sleep(1);
	measure();

	for (int iter = 1;iter <= LASER_CALIBRATION_NUM || tot < SIZE;iter++) {
		measure();
		for (int i = 0;i < SIZE;i++) printf("%d ", dist[i]);
		printf("\n");
		for (int i = 0;i < SIZE;i++) 
			if (dist[i] < 2000) {
				if (cnt[i] == 0) tot++;

				cnt[i]++;
				base[i] += dist[i];
			} else
				break;
	}
	for (int i = 0;i < SIZE;i++) base[i] /= cnt[i];
	printf("========================CALIBRATION============================\n");
	for (int i = 0;i < SIZE;i++) printf("%d ", base[i]);
	printf("\n===============================================================\n");

	for (int iter = 1;;iter++) {
		measure();

		std::stringstream ss;
		ss << "L";
		bool tf = false;
		for (int i = 0;i <= (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE;i++)
			if (dist[i] >= 2000 || tf) {
				tf = true;
				ss << "0 ";
			} else
				ss << dist[i] - base[i] << " ";
		submit("proxy.sock", ss.str().c_str());

	}*/

	close(servofd);
	close(laserfd);
	return 0;
}

