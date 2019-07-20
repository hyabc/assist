#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <map>
#include <fstream>
#include <sstream>
#include "curl/curl.h"
#include "jsoncpp/json/json.h"
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
int processPOSTRequest(char* text) {
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
    processPOSTRequest("一个进程的PID为10000，那么/proc下会有一个名字为10000的文件夹，其中包含有该进程的几乎所有信息：其中/proc/10000/cmdline文件中保存了启动该进程时使用的命令行。	由于刚才的进程是通过./test运行的，因此只要遍历/proc下的文件夹，如果发现某个文件夹中的cmdline文件内容为./test，则该文件夹的名字即为进程的PID，代码如下：在我的另外一个篇博客【Linux下C语言开发（信号signal处理机制）】中需要测试系统调用kill来向指定进行号发送指定的信号，在同一个测试文件很容易获取当前进程的pid，只需调用getpid()函数就可获取当前进程的pid。但是，如果要获取非当前进程的pid，那应该如何获取呢？即我们需要在Linux C 程序中，已知其他进程的名字，来获取其进程的pid。此时此刻我只能百度了，上网百度，找到两种可行的方法：1、通过popen创建一个管道，执行shell命令并得到返回结果 2、通过搜素/proc文件夹下的文件内容，得到进程PID（这里也可以学习下Linux C中如何读取一个文件夹中的内容）为了方便测试，随便创建l一个progress.c文件，文件内容如下：");
    curl_global_cleanup();
    return 0;
}
