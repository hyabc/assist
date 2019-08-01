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

#define abs(x) ({(x) > 0 ? (x) : -(x);})

int position_state;
/* position_state: 0 : on the road,
					1: approaching roadinter,		(deleted)
					2: away from roadinter
					3: at roadinter*/

char msg[MAXBUF], response[MAXBUF];
#define MIN_STAIRCASE_HEIGHT 100
#define EPS 10
#define MIN_FRONT_DISTANCE 100
#define INF 100000000
#define ROAD_THRESH_LARGE 50
#define ROAD_THRESH_SMALL 30
const double PI = acos(-1.0);

namespace laser {

	int x[MAXBUF];
	int height;

	void solve(std::string line) {
		std::stringstream ss(line);

		for (int i = 0;i < SIZE;i++) 
			ss >> x[i];

/*		for (int i = 0;i < size;i++) 
			printf("%f ", acos(x[0] / x[i]) / PI * 180.0);
		printf("\n");
		for (int i = 0;i < size;i++) 
			printf("%f ", (x[0] / x[i]));
		printf("\n");*/


		for (int i = 0;i < SIZE;i++)
			if (abs(x[i]) > MIN_STAIRCASE_HEIGHT * cos((double)(i * DELTA_ANGLE) * PI / 180.0) {
				puts("ALERT!!!");
				printf("\n");

				double delta = x[i];
//				if (abs(delta - height) < EPS) return;
				height = delta;
				if (height < 0) 
					sprintf(response, "!向上台阶");//, 0.001 * a[0] * tan((double)(DELTA_ANGLE) * i * PI / 180.0));
				else
					sprintf(response, "!向下台阶");//, 0.001 * a[0] * tan((double)(DELTA_ANGLE) * i * PI / 180.0));
				submit("speech.sock", response);
				return;
			}
	}
}

namespace ultrasonic {

	int a[3];
	bool left, middle, right;
	void solve(std::string line) {

		if (position_state == 3) return;

		std::stringstream ss(line);
		for (int i = 0;i < 3;i++) {
			ss >> a[i];
			if (a[i] == -1) a[i] = INF;
		}

		left = (a[0] < MIN_FRONT_DISTANCE);
		middle = (a[1] < MIN_FRONT_DISTANCE);
		right = (a[2] < MIN_FRONT_DISTANCE);
		bool left = a[0] < MIN_FRONT_DISTANCE, middle = a[1] < MIN_FRONT_DISTANCE, right = a[2] < MIN_FRONT_DISTANCE;
//		printf("%d %d %d\n", a[0], a[1], a[2]);

		if (middle) {
			sprintf(response, "!有障碍");
			submit("speech.sock", response);
			return;
		} else if (left) {
			sprintf(response, " 左障碍");
			submit("speech.sock", response);
			return;
		} else if (right) {
			sprintf(response, " 右障碍");
			submit("speech.sock", response);
			return;
		}
	}
}

namespace position {

	double heading, roadinter_direction;
	char roadinter_direction_str[MAXBUF], roadinter_name1[MAXBUF], roadinter_name2[MAXBUF], road_name[MAXBUF];
	double roadinter_distance;

	double convert1(char* str) {
		if (strcmp(str, "北") == 0) return 0;
		if (strcmp(str, "东北") == 0) return 45;
		if (strcmp(str, "东") == 0) return 90;
		if (strcmp(str, "东南") == 0) return 135;
		if (strcmp(str, "南") == 0) return 180;
		if (strcmp(str, "西南") == 0) return 225;
		if (strcmp(str, "西") == 0) return 270;
		if (strcmp(str, "西北") == 0) return 315;
//		printf("ALERT: direction=%s\n", str);
	}

	const char* convert2(double angle) {
		if (angle < 23) return "北";
		if (angle < 68) return "东北";
		if (angle < 113) return "东";
		if (angle < 158) return "东南";
		if (angle < 203) return "南";
		if (angle < 248) return "西南";
		if (angle < 293) return "西";
		if (angle < 338) return "西北";
		return "北";
	}

	void solve(std::string line) {

		std::stringstream ss(line);
		ss >> heading >> road_name >>
			roadinter_direction_str >> roadinter_distance >> roadinter_name1 >> roadinter_name2;
		roadinter_direction = convert1(roadinter_direction_str);

		if (roadinter_distance > ROAD_THRESH_LARGE) {
			position_state = 0;
			return;
		}
		if (roadinter_distance < ROAD_THRESH_SMALL) {
			if (position_state != 3) {
				position_state = 3;
				sprintf(response, "!你在%s%s路口", roadinter_name1, roadinter_name2);
				submit("speech.sock", response);
			}
			return;
		}
/*		if (abs(heading - roadinter_direction) < 90 || abs(heading - roadinter_direction) > 270) {
			if (position_state != 1) {
				position_state = 1;
				sprintf(response, " 前方%d米是%s%s路口", (int)(round(roadinter_distance)), roadinter_name1, roadinter_name2);
				submit("speech.sock", response);
			}
		} else {*/
			if (position_state != 2) {
				position_state = 2;
				sprintf(response, " 你所在%s路", road_name);
				submit("speech.sock", response);
			}
//		}
	}
}

namespace monitor {

	int volt;

	void solve(std::string line) {
		std::stringstream ss(line);
		ss >> volt;

		if (volt < 10500) {
			sprintf(response, " 电池电量低");
			submit("speech.sock", response);
		} else if (volt < 10000) {
			sync();
			system("sudo poweroff");
		}
	}
}

namespace vision {

	int status;
	void solve(std::string line) {
	
		if (position_state != 3) {
			status = -1;
			return;
		}

		int cur;
		std::stringstream ss(line);
		ss >> cur;

		if (cur == -1) return;

		if (cur != status) {
			if (cur == 0) {
				if (status != 1) {
					sprintf(response, "!红灯");
					submit("speech.sock", response);
				}
			}
			if (cur == 1) {
				sprintf(response, "!绿灯");
				submit("speech.sock", response);
			}
			status = cur;
		}
	}
}

int main() {
	unlink("proxy.sock");

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "proxy.sock");

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(sockfd, 10);

	laser::height = 0;
	position_state = 0;

	while (1) {
		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		int len = recv(clientfd, msg, MAXBUF, 0);
		close(clientfd);

		for (int i = 0;i < len;i++) putchar(msg[i]);
		putchar('\n');

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

			case 'M':
				monitor::solve(line);
				break;

			case 'V':
				vision::solve(line);
				break;
		}

	}
	close(sockfd);
	return 0;
}
