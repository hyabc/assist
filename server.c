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
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	addr.sin_port=htons(8888);
	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {perror("Error in Binding");return -1;}

	if (!fork()) execlp("python3", "python3", "distance.py", NULL);

	while (1) {
		struct sockaddr_in new_addr;
		int new_addr_size = sizeof(new_addr);
		if (recvfrom(sockfd, msg, MAXN, 0, (struct sockaddr *)&new_addr, &new_addr_size) < 0) {perror("Error in Receiving");return -1;}
		printf("%s\n", msg);
//		if (!fork()) execlp("aws", "aws", "polly", "synthesize-speech", "--output-format", "pcm", "--voice-id", "Zhiyu", "--text", msg, "voice", NULL);
//		wait(NULL);
//		if (!fork()) execlp("aplay", "aplay", "voice", NULL);
//		wait(NULL);
	}
	close(sockfd);
	return 0;
}
