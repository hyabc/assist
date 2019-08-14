#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>
#include <termios.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <stdio.h>
extern "C" {
#include "assist.h"
}

char buf[MAXBUF], msg[MAXBUF];
int serialfd;
int dist[SIZE], base[SIZE];

int serialport_init(const char* serialport) {
	struct termios toptions;
	int fd = open(serialport, O_RDWR | O_NONBLOCK );

	tcgetattr(fd, &toptions);
	speed_t brate = B115200;
	cfsetispeed(&toptions, brate);
	cfsetospeed(&toptions, brate);

	toptions.c_cflag &= ~PARENB;
	toptions.c_cflag &= ~CSTOPB;
	toptions.c_cflag &= ~CSIZE;
	toptions.c_cflag |= CS8;
	toptions.c_cflag &= ~CRTSCTS;


	toptions.c_cflag |= CREAD | CLOCAL;
	toptions.c_iflag &= ~(IXON | IXOFF | IXANY);

	toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
	toptions.c_oflag &= ~OPOST; 

	toptions.c_cc[VMIN]  = 0;
	toptions.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &toptions);
	tcsetattr(fd, TCSAFLUSH, &toptions);

	return fd;
}

void serialport_write(int fd, int x) {
	sprintf(buf, "%d\n", x);
	write(fd, buf, strlen(buf));
	tcflush(fd, TCIOFLUSH);
}

void serialport_readline(int fd) {
	int pos = 0;
	while (true) {
		read(fd, buf + pos, 1);
		if (buf[pos] == '\n') 
			break;
		pos++;
	}
	buf[pos] = 0;
}

int main() {
	serialfd = serialport_init("/dev/ttyACM1");

	memset(base, 0, sizeof(base));

	int countdown = LASER_CALIBRATION_NUM;
	while (true) {
		serialport_readline(serialfd);

		std::stringstream rss(buf);
		for (int i = 0;i < SIZE;i++) {
			rss >> dist[i];
			printf("%d ", dist[i]);
		}
		printf("\n");

		if (countdown--) {
			for (int i = 0;i < SIZE;i++) base[i] += dist[i];

			if (!countdown) {
				for (int i = 0;i < SIZE;i++) base[i] /= LASER_CALIBRATION_NUM;
				printf("========================CALIBRATION============================\n");
				for (int i = 0;i < SIZE;i++) printf("%d ", base[i]);
				printf("\n===============================================================\n");
			}
		} else {
			std::stringstream wss;
			wss << "L";
			for (int i = 0;i < SIZE;i++) wss << dist[i] - base[i] << " ";
			submit("proxy.sock", wss.str().c_str());
		}
	}

	close(serialfd);
	return 0;
}

