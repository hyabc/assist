#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X sensor = Adafruit_VL53L0X();
void setup() {
	Serial.begin(115200);
	while (!Serial) delay(1);
  sensor.begin();
}
void loop() {
	VL53L0X_RangingMeasurementData_t dist;
	sensor.rangingTest(&dist, false);
	if (dist.RangeStatus != 4)
		Serial.println(dist.RangeMilliMeter);
	else
		Serial.println(-1);
	delay(10);
}
