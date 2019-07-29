#include <Servo.h>

int angle;
Servo servo;

void setup() {
	Serial.begin(115200);
	servo.write(90);
	servo.attach(11);
}

void loop() {
	angle = Serial.read();
	servo.write(angle);
}
