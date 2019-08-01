const int trig[3] = {3, 5, 7}, echo[3] = {4, 6, 8};
int i, j;
double duration, dist;
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

		dist = (double)(pulseIn(echo[x], HIGH, 100000)) / 2 / 29.1;
		if (dist > 1.0) 
			distance += dist;
		else
			distance += 1000000;

		delayMicroseconds(100);
	}
	distance /= 10.0;
}
void loop() {
	Serial.print('U');
	for (i = 0;i < 3;i++) {
		measure(i);
		if (distance > 1.0)
			Serial.print((long)(round(distance)));
		else 
			Serial.print(-1);
		Serial.print(' ');
		delay(10);
	}
	Serial.println();
}
