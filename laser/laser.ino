#define MIN_ANGLE 90
#define MAX_ANGLE 150
#define DELTA_ANGLE 1
#define SIZE ((MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1)
#define OFFSET 0

#include <Servo.h>
#include<SoftwareSerial.h>

const int HEADER = 0x59;

SoftwareSerial sensor(2,3);
Servo servo;

int distance, strength, checksum, i, angle, success;
int data[9];

void setup() {
	Serial.begin(115200);
	sensor.begin(115200);
	servo.attach(11);
}

void measure() {
	success = false;
	while (!success) {
		if (sensor.available()) {
			if (sensor.read() == HEADER) {
				data[0] = HEADER;
				if (sensor.read() == HEADER) {
					data[1] = HEADER;

					for (i = 2;i <= 8;i++)
						data[i] = sensor.read();

					checksum = 0;
					for (i = 0;i <= 7;i++)
						checksum += data[i];

					if(data[8] == (checksum & 0xff)) {
						distance = data[2] + data[3] * 256;
						strength = data[4] + data[5] * 256;
						success = true;
					}
				}
			}
		}
	}
}

void loop() {
	Serial.print('L');
	for (angle = MIN_ANGLE;angle <= MAX_ANGLE;angle += DELTA_ANGLE) {
		servo.write(angle + OFFSET);
		delay(5);
		measure();
		measure();
		Serial.print(distance);
		Serial.print(' ');
	}
	for (angle = MAX_ANGLE;angle >= MIN_ANGLE;angle -= DELTA_ANGLE) {
		servo.write(angle + OFFSET);
		delay(5);
		measure();
	}
	Serial.println();
}
