#include <stdio.h> 
#include <math.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h> 
#include <stddef.h> 
#include <string.h>
using namespace std;
#define MAXBUF 10000
char msg[MAXBUF];
int len;
#define MIN_ANGLE 90
#define MAX_ANGLE 150
#define DELTA_ANGLE 5
const double PI = acos(-1.0);
void laser_func() {
	string line(msg + 1, len - 1);
	stringstream ss(line);
	int size = (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1;
	double a[100];
	for (int i = 0;i < size;i++) {
		double alpha = (double)(DELTA_ANGLE) * i * PI / 180.0, dist;
		ss >> dist;
		a[i] = (double)(dist) * cos(alpha);
		printf("%f ", a[i]);
	}
	printf("\n");

}
int main() {
	unlink("assist.sock");

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "assist.sock");
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(sockfd, 10);

	if (!fork()) execlp("python3", "python3", "launch_speech.py", NULL);

	if (!fork()) execlp("python3", "python3", "laser_distance.py", NULL);
	//if (!fork()) execl("position", "position", NULL);
	//if (!fork()) execl("distance", "distance", NULL);
	//if (!fork()) execl("voltage", "voltage", NULL);
	//if (!fork()) execlp("python3", "python3", "vision.py", NULL);
	//if (!fork()) execlp("python3", "python3", "test.py", NULL);
	sleep(1);

	struct sockaddr_un speech_addr;
	memset(&speech_addr, 0, sizeof(speech_addr));
	speech_addr.sun_family = AF_UNIX;
	strcpy(speech_addr.sun_path, "speech.sock");

	while (1) {
		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		len = recv(clientfd, msg, MAXBUF, 0);

		for (int i = 0;i < len;i++) putchar(msg[i]);
		putchar('\n');
		close(clientfd);

		switch (msg[0]) {
			case 'L':
				laser_func();
				break;
		}

/*		int speech_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		connect(speech_sockfd, (struct sockaddr *)&speech_addr, sizeof(speech_addr));
		send(speech_sockfd, msg, len, 0);
		close(speech_sockfd);*/
	}
	close(sockfd);
	return 0;
}
