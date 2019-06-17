#include <stdio.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h> 
#include <stddef.h> 
#include <string.h>
#define MAXN 10000
char msg[MAXN + 10];
int main() {
	unlink("assist.sock");
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "assist.sock");
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(sockfd, 10);

	//if (!fork()) execlp("python3", "python3", "distance.py", NULL);
	if (!fork()) execl("position", "position", NULL);
	if (!fork()) execl("distance", "distance", NULL);
	if (!fork()) execl("voltage", "voltage", NULL);
	if (!fork()) execlp("python3", "python3", "vision.py", NULL);
	//if (!fork()) execlp("python3", "python3", "test.py", NULL);

	while (1) {
		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		size_t len = recv(clientfd, msg, MAXN, 0);
		for (int i = 0;i < len;i++) putchar(msg[i]);
		putchar('\n');
		putchar('\n');
		close(clientfd);
//		if (!fork()) execlp("aws", "aws", "polly", "synthesize-speech", "--output-format", "pcm", "--voice-id", "Zhiyu", "--text", msg, "voice", NULL);
//		wait(NULL);
//		if (!fork()) execlp("aplay", "aplay", "voice", NULL);
//		wait(NULL);
	}
	close(sockfd);
	return 0;
}
