#include <stdio.h>
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
struct string {char* str;size_t size;};
struct string result;

size_t writeToString(void* content, size_t size, size_t nmemb, void *pointer) {
	size_t realsize = size * nmemb;
	struct string *mem = (struct string*)(pointer);
	mem->str = (char*)realloc(mem->str, mem->size + realsize + 1);
	memcpy(&(mem->str[mem->size]), content, realsize);
	mem->size += realsize;
	mem->str[mem->size] = 0;
	return realsize;
}

char revGeoRequest[MAXBUF], response[MAXBUF], roadinter_direction[MAXBUF], roadinter_distance[MAXBUF], roadinter_name1[MAXBUF], roadinter_name2[MAXBUF], 
msg[MAXBUF], coordinates[MAXBUF], heading[MAXBUF], road_name[MAXBUF];

int main() {
	curl_global_init(CURL_GLOBAL_ALL);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(5678);

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	while (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		puts("BIND FAILED");
		sleep(1);
	}
	listen(sockfd, 10);

	while (1) {

		struct sockaddr_un new_addr;
		socklen_t new_addr_size = sizeof(new_addr);
		int clientfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_size);
		int len = recv(clientfd, msg, MAXBUF, 0);
		close(clientfd);

		std::string line(msg, len);
		std::stringstream ss(line);

		ss >> coordinates >> heading >> road_name;

		result.str = (char*)malloc(1);
		result.size = 0;

		CURL *curl = curl_easy_init();
		sprintf(revGeoRequest, "https://restapi.amap.com/v3/geocode/regeo?output=xml&location=%s&key=%s&extensions=all", coordinates, getenv("amapkey"));
		curl_easy_setopt(curl, CURLOPT_URL, revGeoRequest);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
		curl_easy_perform(curl);

//					printf("%s  \n", result.str);
		xmlDocPtr doc = xmlParseMemory(result.str, result.size);
		xmlNodePtr cur = xmlDocGetRootElement(doc);
		while (cur) {
			if (!xmlStrcmp(cur->name, (const xmlChar *)"response")) {
				xmlNodePtr cur1 = cur->xmlChildrenNode;
				while (cur1) {
					if (!xmlStrcmp(cur1->name, (const xmlChar *)"regeocode")) {
						xmlNodePtr cur2 = cur1->xmlChildrenNode;
						while (cur2) {

							if (!xmlStrcmp(cur2->name, (const xmlChar *)"roadinters")) {
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
							}
/*
							if (!xmlStrcmp(cur2->name, (const xmlChar *)"roads")) {
								xmlNodePtr cur3 = cur2->xmlChildrenNode;
								while (cur3) {
									if (!xmlStrcmp(cur3->name, (const xmlChar *)"road")) {
										xmlNodePtr cur4 = cur3->xmlChildrenNode;
										while (cur4) {
											if (!xmlStrcmp(cur4->name, (const xmlChar *)"direction"))
												strcpy(road_direction, cur4->children->content);

											if (!xmlStrcmp(cur4->name, (const xmlChar *)"distance"))
												strcpy(road_distance, cur4->children->content);

											if (!xmlStrcmp(cur4->name, (const xmlChar *)"name"))
												strcpy(road_name, cur4->children->content);

											cur4 = cur4->next;
										}
										break;
									}
									cur3 = cur3->next;
								}
							} */

							cur2 = cur2->next;
						}
					}
					cur1 = cur1->next;
				}
			}
			cur = cur->next;
		}
		curl_easy_cleanup(curl);
		free(result.str);
		xmlFreeDoc(doc);

//					printf("(%lf,%lf)-> ", gps_data.fix.longitude, gps_data.fix.latitude);
//					printf("HEADING: %lf, SPEED %lf\n", gps_data.fix.track, gps_data.fix.speed); /**< GNSS course angle [degree] (0 => north, 90 => east, 180 => south, 270 => west, no negative values). */

		sprintf(response, "P%s %s %s %s %s %s", heading, road_name, roadinter_direction, roadinter_distance, roadinter_name1, roadinter_name2);
		submit("proxy.sock", response);
	printf("%s\n", response);

	}

	xmlCleanupParser();
	curl_global_cleanup();

	return 0;
}

