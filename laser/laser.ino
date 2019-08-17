#include <SoftwareSerial.h>
const int HEADER=0x59;

SoftwareSerial sensor(2,3);
int dist, strength, checksum, i, data[9];

void setup() {
	Serial.begin(115200);
	sensor.begin(115200);
}

void loop() {
	if (sensor.available()) {
		if (sensor.read() == HEADER) {
			data[0] = HEADER;
			if (sensor.read() == HEADER) {
				data[1] = HEADER;

				for (i = 2;i < 9;i++)
					data[i] = sensor.read();

				checksum = 0;
				for(i = 0;i < 8;i++)
					checksum += data[i];

				if (data[8] == (checksum & 0xff)) {
					dist = data[2] + data[3] * 256;
					strength = data[4] + data[5] * 256;
					Serial.print(dist);
					Serial.print(' ');
					Serial.print(strength);
					Serial.println();
				}
			}
		}
	}
}
