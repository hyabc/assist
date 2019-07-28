#include <Wire.h>
#include <VL53L0X.h>
#include <Servo.h>
VL53L0X sensor;
Servo servo;
int angle, i;
#define MAX_ANGLE 150
#define MIN_ANGLE 90
#define DELTA_ANGLE 5
void setup() {
	Serial.begin(115200);
	Wire.begin();
	sensor.init();
	sensor.setTimeout(500);
	sensor.startContinuous();
	servo.attach(11);
}
long d[(MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1];
void loop() {
	Serial.print('L');
	for (angle = MIN_ANGLE;angle <= MAX_ANGLE;angle += DELTA_ANGLE) {
		servo.write(angle);
		delay(100);
		d[(angle - MIN_ANGLE) / DELTA_ANGLE] = sensor.readRangeContinuousMillimeters();
	}
	for (i = 0;i <= (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE;i++) {
		Serial.print(d[i]);
		Serial.print(' ');
	}
	Serial.println();
	delay(50);

	Serial.print('L');
	for (angle = MAX_ANGLE;angle >= MIN_ANGLE;angle -= DELTA_ANGLE) {
		servo.write(angle);
		delay(100);
		d[(angle - MIN_ANGLE) / DELTA_ANGLE] = sensor.readRangeContinuousMillimeters();
	}
	for (i = 0;i <= (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE;i++) {
		Serial.print(d[i]);
		Serial.print(' ');
	}
	Serial.println();
	delay(50);
}
