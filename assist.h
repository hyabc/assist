#ifndef __ASSIST_H__
#define __ASSIST_H__

#define MIN_ANGLE 90
#define MAX_ANGLE 150
#define DELTA_ANGLE 2
#define SIZE ((MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1)

#define MAXBUF 1000

#define OFFSET 20

#define MIN_STAIRCASE_HEIGHT 100

#define MIN_FRONT_DISTANCE 200
#define MIN_SIDE_DISTANCE 100
#define INF 100000000

#define ROAD_THRESH_LARGE 40
#define ROAD_THRESH_SMALL 30

#define LASER_CALIBRATION_NUM 5

#define TRAFFIC_THRESH 1.3

#define MAX_Q1_SIZE 3
#define MAX_Q2_SIZE 3

#define MAX_SPEECH_TYPE 10

void submit(const char*, const char*);

#endif

