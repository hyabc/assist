#include <stdlib.h>
#include <termios.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <stdio.h>
#include "assist.h"
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

#define MAX_ANGLE 150
#define MIN_ANGLE 90
#define DELTA_ANGLE 2

char buf[MAXBUF];
int dist[(MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1];

/*void WaitDataReady(VL53L0X_DEV dev) {
	uint8_t isready = 0;
	int cnt = 0;
	do {
		VL53L0X_GetMeasurementDataReady(dev, &isready);
		if (isready == 0x01) 
			break;

		cnt++;
		VL53L0X_PollingDelay(dev);

	} while (cnt < VL53L0X_DEFAULT_MAX_LOOP);
}

void WaitStopCompleted(VL53L0X_DEV dev) {
	uint32_t isstopcompleted = 0;
	int cnt = 0;
	do {
		VL53L0X_GetStopCompletedStatus(dev, &isstopcompleted);
		if (isstopcompleted == 0x00)  
			break;

		cnt++;
		VL53L0X_PollingDelay(dev);
	} while (cnt < VL53L0X_DEFAULT_MAX_LOOP);
}*/

void startmeasurement(VL53L0X_Dev_t *devptr) {
	uint32_t refSpadCount;
	uint8_t isApertureSpads, VhvSettings, PhaseCal;

	VL53L0X_StaticInit(devptr);
	VL53L0X_PerformRefCalibration(devptr, &VhvSettings, &PhaseCal);
	VL53L0X_PerformRefSpadManagement(devptr, &refSpadCount, &isApertureSpads);
	VL53L0X_SetDeviceMode(devptr, VL53L0X_DEVICEMODE_SINGLE_RANGING);
}

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


	toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
	toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

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

int main() {

	int serialfd = serialport_init("/dev/ttyACM0");

	VL53L0X_Dev_t sensor;
	sensor.I2cDevAddr = 0x29;
	sensor.fd = VL53L0X_i2c_init("/dev/i2c-1", 0x29);

	VL53L0X_DataInit(&sensor);
	startmeasurement(&sensor);

	VL53L0X_RangingMeasurementData_t measurementdata;

	for (int iter = 1;iter <= 50;iter++) {

		for (int angle = MIN_ANGLE;angle <= MAX_ANGLE;angle += DELTA_ANGLE) {
			serialport_write(serialfd, angle);
			usleep(20000);

			VL53L0X_PerformSingleRangingMeasurement(&sensor, &measurementdata);

			dist[(angle - MIN_ANGLE) / DELTA_ANGLE] = measurementdata.RangeMilliMeter;

		}
		for (int i = 0;i <= (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE;i++) printf("%d ", dist[i]);printf("\n");

		for (int angle = MAX_ANGLE;angle >= MIN_ANGLE;angle -= DELTA_ANGLE) {
			serialport_write(serialfd, angle);
			usleep(20000);

			VL53L0X_PerformSingleRangingMeasurement(&sensor, &measurementdata);

			dist[(angle - MIN_ANGLE) / DELTA_ANGLE] = measurementdata.RangeMilliMeter;

		}
		for (int i = 0;i <= (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE;i++) printf("%d ", dist[i]);printf("\n");

	}


	VL53L0X_i2c_close();
	close(serialfd);
	return 0;
}

