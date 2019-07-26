#include <stdio.h> 
#include <algorithm>
#include <math.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <sys/types.h>
#include <stdlib.h> 
#include <stddef.h> 
#include <string.h>
extern "C" {
#include "assist.h"
}
char msg[MAXBUF], response[MAXBUF];
#define MIN_ANGLE 90
#define MAX_ANGLE 150
#define DELTA_ANGLE 5
#define MIN_STAIRCASE_HEIGHT 50
#define EPS 10
#define MIN_FRONT_DISTANCE 200
#define INF 100000000
const double PI = acos(-1.0);
struct sockaddr_un addr;
namespace laser {
	double a[100];
	double height;
	void solve(std::string line) {
		std::stringstream ss(line);
		int size = (MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1;
		for (int i = 0;i < size;i++) {
			int dist;
			ss >> dist;
			if (dist == -1) dist = INF;
			a[i] = (double)(dist) * cos((double)(DELTA_ANGLE) * i * PI / 180.0);
		}
		for (int i = 1;i < size;i++)
			if (fabs(a[i] - a[0]) > MIN_STAIRCASE_HEIGHT) {
				double delta = a[i] - a[0];
				if (fabs(delta - height) < EPS) return;
				height = delta;
				if (height > 0) 
					sprintf(response, "!前方%.2f米有向下台阶", a[i] * tan((double)(DELTA_ANGLE) * i * PI / 180.0));
				else
					sprintf(response, "!前方%.2f米有向上台阶", a[i] * tan((double)(DELTA_ANGLE) * i * PI / 180.0));
				::submit("speech.sock", response);
				return;
			}
	}
}
namespace ultrasonic {
	int a[3];
	void solve(std::string line) {
		std::stringstream ss(line);
		for (int i = 0;i < 3;i++) {
			ss >> a[i];
			if (a[i] == -1) a[i] = INF;
		}
		bool left = a[0] < MIN_FRONT_DISTANCE, middle = a[1] < MIN_FRONT_DISTANCE, right = a[2] < MIN_FRONT_DISTANCE;
		printf("%d %d %d\n", a[0], a[1], a[2]);
		if (middle) {
			if (left && !right)
				sprintf(response, "!前方有障碍物，向右往前走");
			else if (right && !left)
				sprintf(response, "!前方有障碍物，向左往前走");
			else if (a[0] > a[2]) 
				sprintf(response, "!前方有障碍物，向左往前走");
			else
				sprintf(response, "!前方有障碍物，向右往前走");
			::submit("speech.sock", response);
			return;
		}
	}
}
namespace position {
	double distance;
	char road1[500], road2[500], direction[500];
	void solve(std::string line) {
		std::stringstream ss(line);
		ss >> distance >> road1 >> road2 >> direction;
		sprintf(response, " %s方向%lf米是%s%s路口", direction, distance, road1, road2);
		::submit("speech.sock", response);
	}
}
int main() {
	unlink("assist.sock");

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "assist.sock");

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(sockfd, 10);

	laser::height = 0;
	while (1) {
		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		int len = recv(clientfd, msg, MAXBUF, 0);

		for (int i = 0;i < len;i++) putchar(msg[i]);
		putchar('\n');
		close(clientfd);

		std::string line(msg + 1, len - 1);

		switch (msg[0]) {
			case 'L':
				laser::solve(line);
				break;

			case 'U':
				ultrasonic::solve(line);
				break;

			case 'P':
				position::solve(line);
				break;
		}

	}
	close(sockfd);
	return 0;
}