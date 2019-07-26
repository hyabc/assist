#include <stdio.h> 
#include <math.h>
#include <unistd.h>
#include <stdlib.h> 
#include <stddef.h> 
#include <string.h>
int main() {
	if (!fork()) execl("speech", "speech", NULL);
	sleep(1);
	if (!fork()) execl("proxy", "proxy", NULL);
	sleep(1);
	if (!fork()) execlp("python3", "python3", "adapter1.py", NULL);
	if (!fork()) execlp("python3", "python3", "adapter2.py", NULL);
	if (!fork()) execl("position", "position", NULL);
	if (!fork()) execl("voltage", "voltage", NULL);
	return 0;
}
