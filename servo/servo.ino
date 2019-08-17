#include <Servo.h>

int angle;
Servo servo;

void setup() {
	Serial.begin(115200);
	servo.write(90);
	servo.attach(11);
}

void loop() {
	if (Serial.available()) {
		angle = Serial.parseInt();
		if (angle != 0)
			servo.write(angle);
	}
}
