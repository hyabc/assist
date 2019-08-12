#include <sys/socket.h>
#include <math.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <string.h>
#include "assist.h"

void submit(const char* sockname, const char* msg) {
	struct sockaddr_un addr;	
	memset(&addr, 0, sizeof(addr));	
	addr.sun_family = AF_UNIX;	
	strcpy(addr.sun_path, sockname);

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	send(sockfd, msg, strlen(msg), 0);
	close(sockfd);
}
