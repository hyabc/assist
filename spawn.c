#include <stdio.h> 
#include <math.h>
#include <unistd.h>
#include <stdlib.h> 
#include <stddef.h> 
#include <string.h>
int main() {
	if (!fork()) execlp("python3", "python3", "launch_speech.py", NULL);
	if (!fork()) execlp("python3", "python3", "laser_distance.py", NULL);
	if (!fork()) execlp("python3", "python3", "ultrasonic_distance.py", NULL);
	if (!fork()) execl("position", "position", NULL);
	//if (!fork()) execl("distance", "distance", NULL);
	//if (!fork()) execl("voltage", "voltage", NULL);
	//if (!fork()) execlp("python3", "python3", "vision.py", NULL);
	//if (!fork()) execlp("python3", "python3", "test.py", NULL);
	return 0;
}
