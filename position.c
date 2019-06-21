#include <stdio.h>
#include <libxml/parser.h>
#include <gps.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <curl/curl.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <stddef.h> 
#include <string.h>
struct string {char* str;size_t size;};
size_t writeToString(void* content, size_t size, size_t nmemb, void *pointer) {
	size_t realsize = size * nmemb;
	struct string *mem = (struct string*)(pointer);
	mem->str = realloc(mem->str, mem->size + realsize + 1);
	memcpy(&(mem->str[mem->size]), content, realsize);
	mem->size += realsize;
	mem->str[mem->size] = 0;
	return realsize;
}
char coordinateChangeRequest[500], coordinateChangeResult[500], revGeoRequest[500];
char response[500];

int main() {
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "assist.sock");
	curl_global_init(CURL_GLOBAL_ALL);
	struct gps_data_t gps_data;
	if (gps_open("localhost", "2947", &gps_data) == -1) {
		puts("Cannot open gps");
		return 0;
	}
	gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

	while (1) {
		if (gps_waiting (&gps_data, 500)) {
			if (gps_read(&gps_data) == -1) {
				puts("Read failed");
			} else {
				if (gps_data.status == STATUS_FIX && (gps_data.fix.mode == MODE_2D || gps_data.fix.mode == MODE_3D)) {
//					printf("(%lf,%lf)-> ", gps_data.fix.longitude, gps_data.fix.latitude);
					struct string result;
					result.str = malloc(1);
					result.size = 0;
					CURL *curl = curl_easy_init();
					sprintf(coordinateChangeRequest, "https://restapi.amap.com/v3/assistant/coordinate/convert?locations=%f,%f&coordsys=gps&output=xml&key=%s", gps_data.fix.longitude, gps_data.fix.latitude, getenv("amapkey"));
					curl_easy_setopt(curl, CURLOPT_URL, coordinateChangeRequest);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
					curl_easy_perform(curl);
	//				printf("%s  ", result.str);
					xmlDocPtr doc = xmlParseMemory(result.str, result.size);
					xmlNodePtr cur = xmlDocGetRootElement(doc);
					while (cur) {
						if (!xmlStrcmp(cur->name, (const xmlChar *)"response")) {
							xmlNodePtr cur1 = cur->xmlChildrenNode;
							while (cur1) {
								if (!xmlStrcmp(cur1->name, (const xmlChar *)"locations")) {
									strcpy(coordinateChangeResult, cur1->children->content);
//									printf("%s\n", coordinateChangeResult);
								}
								cur1 = cur1->next;
							}
						}
						cur = cur->next;
					}
					xmlFreeDoc(doc);
					curl_easy_cleanup(curl);
					free(result.str);
					sprintf(revGeoRequest, "https://restapi.amap.com/v3/geocode/regeo?output=xml&location=%s&key=%s&extensions=all", coordinateChangeResult, getenv("amapkey"));
					result.str = malloc(1);
					result.size = 0;
					curl = curl_easy_init();
					curl_easy_setopt(curl, CURLOPT_URL, revGeoRequest);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
					curl_easy_perform(curl);
					doc = xmlParseMemory(result.str, result.size);
					cur = xmlDocGetRootElement(doc);
					response[0] = 0;
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
														if (!xmlStrcmp(cur4->name, (const xmlChar *)"direction")) {
															strcat(response, "方向");
															strcat(response, cur4->children->content);
															strcat(response, ", ");
														}
														if (!xmlStrcmp(cur4->name, (const xmlChar *)"distance")) {
															strcat(response, cur4->children->content);
															strcat(response, "米, ");
														}
														if (!xmlStrcmp(cur4->name, (const xmlChar *)"first_name")) {
															strcat(response, cur4->children->content);
															strcat(response, ", ");
														}
														if (!xmlStrcmp(cur4->name, (const xmlChar *)"second_name")) {
															strcat(response, cur4->children->content);
															strcat(response, ", ");
														}
														cur4 = cur4->next;
													}
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
					curl_easy_cleanup(curl);
					free(result.str);
					xmlFreeDoc(doc);
					//printf("%s\n", response);
					int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
					connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
					send(sockfd, response, strlen(response), 0);
					close(sockfd);
				} else {
					printf("GPS not fixed\n");
				}
			}
		}
		sleep(0.5);
	}
	gps_stream(&gps_data, WATCH_DISABLE, NULL);
	gps_close(&gps_data);
	xmlCleanupParser();
	curl_global_cleanup();
	return 0;
}
