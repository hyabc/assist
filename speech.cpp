#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <map>
#include <fstream>
#include <sstream>
#include "curl/curl.h"
#include "jsoncpp/json/json.h"
#define MAXBUF 10000
char msg[MAXBUF];
using namespace std;
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
void generate(const char* text) {
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://nls-gateway.cn-shanghai.aliyuncs.com/stream/v1/tts");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	struct curl_slist* headers = curl_slist_append(headers, "Content-Type:application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	Json::Value root;
	Json::FastWriter writer;
	root["appkey"] = getenv("appKey");
	root["token"] = getenv("token");
	root["text"] = text;
		printf("%s\n", text);
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
		stream.open("voice.mp3", ios::out | ios::binary);
		stream.write(bodyContent.c_str(), bodyContent.size());
		stream.close();
	}
}
int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_ALL);
	unlink("speech.sock");
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "speech.sock");
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(sockfd, 10);
	int lastPID = -1;
	while (1) {
		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		size_t len = recv(clientfd, msg, MAXBUF, 0);
		close(clientfd);
		generate(msg + 1);
		if (msg[0] == '!' && lastPID != -1) kill(lastPID, SIGKILL);
		int PID = fork();
		if (PID == 0)
			execlp("play", "play", "voice.mp3", NULL);
		else
			lastPID = PID;
	}
    curl_global_cleanup();
    return 0;
}
