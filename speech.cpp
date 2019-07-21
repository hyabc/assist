#include <iostream>
#include <mutex>
#include <pthread.h>
#include <uuid/uuid.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <string>
#include <queue>
#include <cstring>
#include <map>
#include <fstream>
#include <sstream>
#include "curl/curl.h"
#include "jsoncpp/json/json.h"
using namespace std;
#define MAXBUF 10000
#define MAX_Q1_SIZE 3
#define MAX_Q2_SIZE 3
char msg[MAXBUF];
pthread_mutex_t mutex1, mutex2, mutex3;
int playPID;
struct node {
	char name[50];
	string content;
	bool important;
	node(char* msg): content(msg + 1), important(msg[0] == '!') {
		char buf[50];
		uuid_t uuid;
		uuid_generate(uuid);
		uuid_unparse(uuid, buf);
		name[0] = 0;
		strcat(name, "voice/");
		strcat(name, buf);
		strcat(name, ".mp3");
		uuid_clear(uuid);
	}
};
std::queue<node> q1, q2;
size_t header_func(void* ptr, size_t size, size_t nmemb, void* dest) {
	map<string, string> *headers = (map<string, string>*)dest;
	string line((char*)ptr);
	string::size_type pos = line.find(':');
	if (pos != line.npos) {
		string name = line.substr(0, pos);
		string value = line.substr(pos + 2);
		size_t p = 0;
		if ((p = value.rfind('\r')) != value.npos)
			value = value.substr(0, p);
		headers->insert(make_pair(name, value));
	}
	return size * nmemb;
}
size_t body_func(void* ptr, size_t size, size_t nmemb, void* dest) {
	size_t len = size * nmemb;
	string* str = (string*)dest;
	(*str).append(string((char*)(ptr), (char*)(ptr) + len));
	return len;
}
void generate(const node& x) {
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://nls-gateway.cn-shanghai.aliyuncs.com/stream/v1/tts");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	struct curl_slist* headers = curl_slist_append(headers, "Content-Type:application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	Json::Value root;
	Json::FastWriter writer;
	root["appkey"] = getenv("appKey");
	root["token"] = getenv("token");
	root["text"] = x.content;
	root["format"] = "mp3";
	root["sample_rate"] = "16000";
	root["voice"] = "ruoxi";
	// root["volume"] = 50;
	// root["speech_rate"] = 0;
	// root["pitch_rate"] = 0;
	string task = writer.write(root);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, task.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, task.length());
	map<string, string> responseHeaders;
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_func);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);
	string bodyContent = "";
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, body_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bodyContent);
	curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	map<string, string>::iterator iter = responseHeaders.find("Content-Type");
	if (iter != responseHeaders.end() && iter->second.compare("audio/mpeg") == 0) {
		ofstream stream;
		stream.open(x.name, ios::out | ios::binary);
		stream.write(bodyContent.c_str(), bodyContent.size());
		stream.close();
	}
}
void* thread_func1(void* arg) {
	while (1) {
		pthread_mutex_lock(&mutex1);
		int size = q1.size();
		pthread_mutex_unlock(&mutex1);
		if (size > 0) {
			pthread_mutex_lock(&mutex1);
			node x = q1.front();
			//printf("1 - %s\n", x.name);
			q1.pop();
			pthread_mutex_unlock(&mutex1);

			generate(x);

			pthread_mutex_lock(&mutex2);
			if (x.important) {
				while (q2.size()) q2.pop();

				pthread_mutex_lock(&mutex3);
				if (playPID != -1) kill(playPID, SIGKILL);
				pthread_mutex_unlock(&mutex3);
			}
			q2.push(x);
			while (q2.size() > MAX_Q2_SIZE) q2.pop();
			pthread_mutex_unlock(&mutex2);
		}
	}
}
void* thread_func2(void* arg) {
	while (1) {
		pthread_mutex_lock(&mutex2);
		int size = q2.size();
		pthread_mutex_unlock(&mutex2);
		if (size > 0) {
			pthread_mutex_lock(&mutex2);
			node x = q2.front();
			q2.pop();
			pthread_mutex_unlock(&mutex2);

			int PID = fork();
			if (!PID)
				execlp("play", "play", x.name, NULL);
			else {
				pthread_mutex_lock(&mutex3);
				playPID = PID;
				pthread_mutex_unlock(&mutex3);
			}
			wait(0);
			unlink(x.name);
		}
	}
}
int main() {
	playPID = -1;
    curl_global_init(CURL_GLOBAL_ALL);
	unlink("speech.sock");
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "speech.sock");
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(sockfd, 10);
	pthread_t thread1, thread2;
	pthread_create(&thread1, NULL, thread_func1, NULL);
	pthread_create(&thread2, NULL, thread_func2, NULL);
	while (1) {
		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		size_t len = recv(clientfd, msg, MAXBUF, 0);
		close(clientfd);
		node x(msg);

		pthread_mutex_lock(&mutex1);
		if (x.important) {while (q1.size()) q1.pop();}
		q1.push(x);
		pthread_mutex_unlock(&mutex1);

//		printf("3 - %s,   size = %d\n", msg, q1.size());
		pthread_mutex_lock(&mutex1);
		while (q1.size() > MAX_Q1_SIZE) q1.pop();
		pthread_mutex_unlock(&mutex1);
	}
    curl_global_cleanup();
    return 0;
}
