#include <iostream>
#include "assist.h"
#include <fcntl.h>
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
#include <semaphore.h> 
#include <map>
#include <fstream>
#include <sstream>
#include "curl/curl.h"
#include "jsoncpp/json/json.h"
using namespace std;

bool type_exist[10];

char msg[MAXBUF];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER, mutex2 = PTHREAD_MUTEX_INITIALIZER, mutex3 = PTHREAD_MUTEX_INITIALIZER;
sem_t sem1, sem2;
int playPID;
struct node {
	char name[50];
	string content;
	int type;
	bool exist;
	node(char* msg, int len): content(msg + 1, len - 1), type(msg[0] - '0') {
		name[0] = 0;
		strcat(name, "voice/");
		exist = true;
		if (content.compare("有障碍") == 0) 
			strcat(name, "mid");
		else if (content.compare("左障碍") == 0)
			strcat(name, "left");
		else if (content.compare("右障碍") == 0)
			strcat(name, "right");
		else if (content.compare("红灯") == 0)
			strcat(name, "red");
		else if (content.compare("绿灯") == 0)
			strcat(name, "green");
		else {
			char buf[50];
			uuid_t uuid;
			uuid_generate(uuid);
			uuid_unparse(uuid, buf);
			strcat(name, buf);

			uuid_clear(uuid);
			exist = false;
		}
		strcat(name, ".mp3");
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
void generate(node* x) {
	if (x->exist) return;

	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://nls-gateway.cn-shanghai.aliyuncs.com/stream/v1/tts");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type:application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	Json::Value root;
	Json::FastWriter writer;
	root["appkey"] = getenv("appKey");
	root["token"] = getenv("token");
	root["text"] = x->content;
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
		stream.open(x->name, ios::out | ios::binary);
		stream.write(bodyContent.c_str(), bodyContent.size());
		stream.close();
	}
}
void* thread_func1(void* arg) {
	while (1) {
		sem_wait(&sem1);

		node* x = 0;

		pthread_mutex_lock(&mutex1);
		if (q1.size() > 0) {
			x = new node(q1.front());
			q1.pop();
		}
		pthread_mutex_unlock(&mutex1);

		if (x != 0) {
			generate(x);

			pthread_mutex_lock(&mutex2);
/*			if (x->important) {
				while (q2.size()) {
					unlink(q2.front().name);
					q2.pop();
				}

				pthread_mutex_lock(&mutex3);
				if (playPID != -1) kill(playPID, SIGKILL);
				pthread_mutex_unlock(&mutex3);
			}*/
			while (q2.size() > MAX_Q2_SIZE) q2.pop();
			q2.push(*x);
			pthread_mutex_unlock(&mutex2);

			sem_post(&sem2);

			delete x;
		}
	}
}
void* thread_func2(void* arg) {
	while (1) {
		sem_wait(&sem2);

		node* x = 0;

		pthread_mutex_lock(&mutex2);
		if (q2.size() > 0) {
			x = new node(q2.front());
			q2.pop();
		}
		pthread_mutex_unlock(&mutex2);

		if (x != 0) {
			std::cout << "NOW PLAY: " << x->content << " with name " << x->name << std::endl;

			int PID = fork();
			if (!PID) {
				execlp("play", "play", x->name, NULL);
			} else /*if (!x->important)*/ {
				pthread_mutex_lock(&mutex3);
				playPID = PID;
				pthread_mutex_unlock(&mutex3);
			}
			wait(0);
			type_exist[x->type] = false;
			//unlink(x->name);
			delete x;
		}
	}
}
int main() {
	memset(type_exist, false, sizeof(type_exist));

	playPID = -1;
	sem_init(&sem1, 0, 0);
	sem_init(&sem2, 0, 0);
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
		int len = recv(clientfd, msg, MAXBUF, 0);
		close(clientfd);

		node x(msg, len);
		if (type_exist[x.type]) {
			printf("exist: %d\n", x.type);
			continue;
		}
		type_exist[x.type] = true;
		
		printf("speak: %s\n", x.content.c_str());

		pthread_mutex_lock(&mutex1);
//		if (x.important) {while (q1.size()) q1.pop();}
		while (q1.size() > MAX_Q1_SIZE) q1.pop();
		q1.push(x);
		pthread_mutex_unlock(&mutex1);

		sem_post(&sem1);
	}
	curl_global_cleanup();
	sem_destroy(&sem1);
	sem_destroy(&sem2);
    return 0;
}
