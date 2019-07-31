#include <stdio.h> 
#include <math.h>
#include <unistd.h>
#include <stdlib.h> 
#include <stddef.h> 
#include <string.h>
int main() {
	if (!fork()) execlp("python3", "python3", "launch_speech.py", NULL);

	sleep(1);

	if (!fork()) execl("proxy", "proxy", NULL);

	sleep(1);
//	if (!fork()) execlp("python3", "python3", "adapter0.py", NULL);
//	if (!fork()) execlp("python3", "python3", "adapter1.py", NULL);
	if (!fork()) execl("monitor", "monitor", NULL);
	if (!fork()) execl("run", "run", NULL);
//	if (!fork()) execl("position", "position", NULL);
//	if (!fork()) execl("vision", "vision", NULL);

	pause();

	return 0;
}
