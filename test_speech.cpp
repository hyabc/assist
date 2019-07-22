#include <stdio.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h> 
#include <stddef.h> 
#include <string.h>
#define MAXBUF 10000
char msg[MAXBUF];
int main() {
	struct sockaddr_un speech_addr;
	memset(&speech_addr, 0, sizeof(speech_addr));
	speech_addr.sun_family = AF_UNIX;
	strcpy(speech_addr.sun_path, "speech.sock");

	while (1) {
		scanf("%s", msg);
		int speech_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		connect(speech_sockfd, (struct sockaddr *)&speech_addr, sizeof(speech_addr));
		send(speech_sockfd, msg, strlen(msg), 0);
		close(speech_sockfd);
	}
	return 0;
}
