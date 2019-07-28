#include <stdio.h>
#include "assist.h"
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
struct string result;

size_t writeToString(void* content, size_t size, size_t nmemb, void *pointer) {
	size_t realsize = size * nmemb;
	struct string *mem = (struct string*)(pointer);
	mem->str = realloc(mem->str, mem->size + realsize + 1);
	memcpy(&(mem->str[mem->size]), content, realsize);
	mem->size += realsize;
	mem->str[mem->size] = 0;
	return realsize;
}

char coordinateChangeRequest[MAXBUF], coordinateChangeResult[MAXBUF], revGeoRequest[MAXBUF];
char response[MAXBUF], roadinter_direction[MAXBUF], roadinter_distance[MAXBUF], roadinter_name1[MAXBUF], roadinter_name2[MAXBUF], 
	road_direction[MAXBUF], road_distance[MAXBUF], road_name[MAXBUF];

int main() {
	curl_global_init(CURL_GLOBAL_ALL);

	struct gps_data_t gps_data;
	gps_open("localhost", "2947", &gps_data);
	gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

	while (1) {
		if (gps_waiting(&gps_data, 500)) {
			if (gps_read(&gps_data) != -1) {
				if (gps_data.status == STATUS_FIX && (gps_data.fix.mode == MODE_2D || gps_data.fix.mode == MODE_3D)) {

					result.str = malloc(1);
					result.size = 0;

					CURL *curl = curl_easy_init();
					sprintf(coordinateChangeRequest, "https://restapi.amap.com/v3/assistant/coordinate/convert?locations=%f,%f&coordsys=gps&output=xml&key=%s", gps_data.fix.longitude, gps_data.fix.latitude, getenv("amapkey"));
					curl_easy_setopt(curl, CURLOPT_URL, coordinateChangeRequest);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
					curl_easy_perform(curl);

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

					result.str = malloc(1);
					result.size = 0;

					curl = curl_easy_init();
					curl_easy_setopt(curl, CURLOPT_URL, revGeoRequest);
					sprintf(revGeoRequest, "https://restapi.amap.com/v3/geocode/regeo?output=xml&location=%s&key=%s&extensions=all", coordinateChangeResult, getenv("amapkey"));
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
					curl_easy_perform(curl);

//					printf("%s  \n", result.str);
					doc = xmlParseMemory(result.str, result.size);
					cur = xmlDocGetRootElement(doc);
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
															strcpy(roadinter_direction, cur4->children->content);

														if (!xmlStrcmp(cur4->name, (const xmlChar *)"distance")) 
															strcpy(roadinter_distance, cur4->children->content);

														if (!xmlStrcmp(cur4->name, (const xmlChar *)"first_name")) 
															strcpy(roadinter_name1, cur4->children->content);

														if (!xmlStrcmp(cur4->name, (const xmlChar *)"second_name")) 
															strcpy(roadinter_name2, cur4->children->content);

														cur4 = cur4->next;
													}
													break;
												}
												cur3 = cur3->next;
											}
										}

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

//					printf("(%lf,%lf)-> ", gps_data.fix.longitude, gps_data.fix.latitude);
//					printf("HEADING: %lf, SPEED %lf\n", gps_data.fix.track, gps_data.fix.speed); /**< GNSS course angle [degree] (0 => north, 90 => east, 180 => south, 270 => west, no negative values). */

					sprintf(response, "P%d %s %s %s %s %s %s %s", (int)(round(gps_data.fix.track)), road_direction, road_distance, road_name, roadinter_direction, roadinter_distance, roadinter_name1, roadinter_name2);
					submit("proxy.sock", response);
				//	printf("%s\n", response);

				} else 
					puts("GPS NOT FIXED");
			}
		}
		sleep(1);
	}

	gps_stream(&gps_data, WATCH_DISABLE, NULL);
	gps_close(&gps_data);
	xmlCleanupParser();
	curl_global_cleanup();

	return 0;
}

