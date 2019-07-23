const int trig[3] = {7, 3, 5}, echo[3] = {8, 4, 6};
int i, j;
double duration, distance;
void setup() {
	Serial.begin(115200);
	while (!Serial) delay(1);
	for (i = 0;i < 3;i++) {
		pinMode(trig[i], OUTPUT);
		pinMode(echo[i], INPUT);
	}
}
void measure(int x) {
	distance = 0.0;
	for (j = 0;j < 10;j++) {
		digitalWrite(trig[x], LOW);
		delayMicroseconds(5);
		digitalWrite(trig[x], HIGH);
		delayMicroseconds(10);
		digitalWrite(trig[x], LOW);
		pinMode(echo[x], INPUT);
		duration = pulseIn(echo[x], HIGH, 100000);
		distance += (duration / 2) / 29.1;
		delayMicroseconds(100);
	}
	distance /= 10.0;
}
void loop() {
	Serial.print('L');
	for (i = 0;i < 3;i++) {
		measure(i);
		Serial.print(distance);
		Serial.print(' ');
	}
	Serial.println();
}
