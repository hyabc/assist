#include "Adafruit_VL53L0X.h"
#include <Servo.h>
Adafruit_VL53L0X sensor = Adafruit_VL53L0X();
VL53L0X_RangingMeasurementData_t dist;
Servo servo;
int angle;
#define MAXANGLE 150
#define MINANGLE 70
#define DELTAANGLE 5
void setup() {
	Serial.begin(115200);
	while (!Serial) delay(1);
	sensor.begin();
	servo.attach(11);
}
void loop() {
	Serial.print('L');
	for (angle = MINANGLE;angle <= MAXANGLE;angle += DELTAANGLE) {
		servo.write(angle);
		delay(50);
		sensor.rangingTest(&dist, false);
		if (dist.RangeStatus != 4)
			Serial.print(dist.RangeMilliMeter);
		else
			Serial.print(-1);
		Serial.print(' ');
	}
	for (angle = MAXANGLE;angle >= MINANGLE;angle -= DELTAANGLE) {
		servo.write(angle);
		delay(50);
	}
//	delay(200);
	Serial.println();
}
