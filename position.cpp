#include <stdio.h>
#include <time.h>
#include <map>
#include <arpa/inet.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <curl/curl.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <libxml/parser.h>
#include <stddef.h> 
#include <string.h>
extern "C" {
#include "assist.h"
}
struct String {char* str;size_t size;} result;
std::map<std::string, clock_t> last;
bool exist_roadinter;
char revGeoRequest[MAXBUF], response[MAXBUF], roadinter_distance[MAXBUF], roadinter_name[MAXBUF], 
	msg[MAXBUF], coordinates[MAXBUF], road_name[MAXBUF], 
	poi_id[MAXBUF], poi_name[MAXBUF], poi_distance[MAXBUF], poi_type[MAXBUF];


size_t writeToString(void* content, size_t size, size_t nmemb, void *pointer) {
	size_t realsize = size * nmemb;
	String *mem = (String*)(pointer);
	mem->str = (char*)realloc(mem->str, mem->size + realsize + 1);
	memcpy(&(mem->str[mem->size]), content, realsize);
	mem->size += realsize;
	mem->str[mem->size] = 0;
	return realsize;
}

void revgeo_solve(const String& result) {
	xmlDocPtr doc = xmlParseMemory(result.str, result.size);
	xmlNodePtr cur = xmlDocGetRootElement(doc);
	while (cur) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"response")) {
			xmlNodePtr cur1 = cur->xmlChildrenNode;
			while (cur1) {
				if (!xmlStrcmp(cur1->name, (const xmlChar *)"regeocode")) {
					xmlNodePtr cur2 = cur1->xmlChildrenNode;
					while (cur2) {

						/*if (!xmlStrcmp(cur2->name, (const xmlChar *)"roadinters")) {
							xmlNodePtr cur3 = cur2->xmlChildrenNode;
							while (cur3) {
								if (!xmlStrcmp(cur3->name, (const xmlChar *)"roadinter")) {
									xmlNodePtr cur4 = cur3->xmlChildrenNode;
									while (cur4) {
										if (!xmlStrcmp(cur4->name, (const xmlChar *)"direction")) 
											strcpy(roadinter_direction, (const char*)cur4->children->content);

										if (!xmlStrcmp(cur4->name, (const xmlChar *)"distance")) 
											strcpy(roadinter_distance, (const char*)cur4->children->content);

										if (!xmlStrcmp(cur4->name, (const xmlChar *)"first_name")) 
											strcpy(roadinter_name1, (const char*)cur4->children->content);

										if (!xmlStrcmp(cur4->name, (const xmlChar *)"second_name")) 
											strcpy(roadinter_name2, (const char*)cur4->children->content);

										cur4 = cur4->next;
									}
									break;
								}
								cur3 = cur3->next;
							}
						}*/

						if (!xmlStrcmp(cur2->name, (const xmlChar *)"roads")) {
							xmlNodePtr cur3 = cur2->xmlChildrenNode;
							while (cur3) {
								if (!xmlStrcmp(cur3->name, (const xmlChar *)"road")) {
									xmlNodePtr cur4 = cur3->xmlChildrenNode;
									while (cur4) {
/*										if (!xmlStrcmp(cur4->name, (const xmlChar *)"direction"))
											strcpy(road_direction, cur4->children->content);

										if (!xmlStrcmp(cur4->name, (const xmlChar *)"distance"))
											strcpy(road_distance, cur4->children->content);*/

										if (!xmlStrcmp(cur4->name, (const xmlChar *)"name"))
											strcpy(road_name, (const char*)cur4->children->content);

										cur4 = cur4->next;
									}
									break;
								}
								cur3 = cur3->next;
							}
						}

						cur2 = cur2->next;
					}
				}
				cur1 = cur1->next;
			}
		}
		cur = cur->next;
	}
	xmlFreeDoc(doc);
}

void poi_solve(const String& result) {
	xmlDocPtr doc = xmlParseMemory(result.str, result.size);
	xmlNodePtr cur = xmlDocGetRootElement(doc);
	while (cur) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"response")) {
			xmlNodePtr cur1 = cur->xmlChildrenNode;
			while (cur1) {
				if (!xmlStrcmp(cur1->name, (const xmlChar *)"pois")) {
					xmlNodePtr cur2 = cur1->xmlChildrenNode;
					while (cur2) {

						if (!xmlStrcmp(cur2->name, (const xmlChar *)"poi")) {
							xmlNodePtr cur3 = cur2->xmlChildrenNode;
							while (cur3) {
								if (!xmlStrcmp(cur3->name, (const xmlChar *)"id")) 
									strcpy(poi_id, (const char*)cur3->children->content);

								if (!xmlStrcmp(cur3->name, (const xmlChar *)"distance")) 
									strcpy(poi_distance, (const char*)cur3->children->content);

								if (!xmlStrcmp(cur3->name, (const xmlChar *)"name")) 
									strcpy(poi_name, (const char*)cur3->children->content);

								if (!xmlStrcmp(cur3->name, (const xmlChar *)"typecode")) 
									strcpy(poi_type, (const char*)cur3->children->content);

								cur3 = cur3->next;
							}
							
							if (strcmp(poi_type, "190302") == 0) {
								if (!exist_roadinter) {
									exist_roadinter = true;
									strcpy(roadinter_distance, poi_distance);
									strcpy(roadinter_name, poi_name);
								}
							} else {
								int dist;
								sscanf(poi_distance, "%d", &dist);
								if (dist < POI_RADIUS) {
									std::string curID(poi_id);
									if (last.count(curID) == 0 || (double)(clock() - last[curID]) / CLOCKS_PER_SEC > POI_SAME_WAIT_SEC) {
										last[curID] = clock();
										sprintf(response, "0距离%s米有%s", poi_distance, poi_name);
										printf("SUBMIT: %s\n", response);
										submit("speech.sock", response);
									}
								}
							}
						}

						cur2 = cur2->next;
					}
				}
				cur1 = cur1->next;
			}
		}
		cur = cur->next;
	}
	xmlFreeDoc(doc);
}

int main() {
	curl_global_init(CURL_GLOBAL_ALL);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(5678);

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	while (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(sockfd);
		puts("BIND FAILED");
		sleep(1);
		sockfd = socket(PF_INET, SOCK_STREAM, 0);
	}
	listen(sockfd, 10);

	while (1) {
		exist_roadinter = false;
		road_name[0] = 0;
		roadinter_distance[0] = 0;
		roadinter_name[0] = 0;

		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		int len = recv(clientfd, msg, MAXBUF, 0);
		close(clientfd);

		std::string line(msg, len);
		std::stringstream ss(line);

		ss >> coordinates;

		//逆地址编码API
		result.str = (char*)malloc(1);
		result.size = 0;
		CURL *curl = curl_easy_init();
		sprintf(revGeoRequest, "https://restapi.amap.com/v3/geocode/regeo?output=xml&location=%s&key=%s&extensions=all", coordinates, getenv("amapkey"));
		curl_easy_setopt(curl, CURLOPT_URL, revGeoRequest);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
		curl_easy_perform(curl);

		//printf("%s  \n", result.str);
		revgeo_solve(result);
		
		curl_easy_cleanup(curl);
		free(result.str);


		//POI查询API
		result.str = (char*)malloc(1);
		result.size = 0;
		curl = curl_easy_init();
		sprintf(revGeoRequest, "https://restapi.amap.com/v3/place/around?output=xml&location=%s&key=%s&extensions=all&types=%s&radius=%d&sortrule=distance", coordinates, getenv("amapkey"), POI_TYPE, POI_SEARCH_RADIUS);
		curl_easy_setopt(curl, CURLOPT_URL, revGeoRequest);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
		curl_easy_perform(curl);

		//printf("%s  \n", result.str);
		poi_solve(result);
		
		curl_easy_cleanup(curl);
		free(result.str);


		if (exist_roadinter)
			sprintf(response, "P%s %s %s", road_name, roadinter_distance, roadinter_name);
		else 
			sprintf(response, "P%s %d", road_name, INF);
			
		submit("proxy.sock", response);

	}

	xmlCleanupParser();
	curl_global_cleanup();

	return 0;
}

