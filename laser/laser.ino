#include <Wire.h>
#include <VL53L0X.h>
#include <Servo.h>
VL53L0X sensor;
VL53L0X_RangingMeasurementData_t dist;
Servo servo;
int angle;
#define MAXANGLE 150
#define MINANGLE 90
#define DELTAANGLE 2
void setup() {
	Serial.begin(115200);
	Wire.begin();
	sensor.init();
	sensor.setTimeout(500);
	sensor.startContinuous();
	servo.attach(11);
}
void loop() {
	Serial.print('L');
	for (angle = MINANGLE;angle <= MAXANGLE;angle += DELTAANGLE) {
		servo.write(angle);
		delay(20);
		Serial.print(sensor.readRangeContinuousMillimeters());
		Serial.print(' ');
	}
	for (angle = MAXANGLE;angle >= MINANGLE;angle -= DELTAANGLE) {
		servo.write(angle);
		delay(10);
	}
	delay(50);
	Serial.println();
}
