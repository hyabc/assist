#ifndef __ASSIST_H__
#define __ASSIST_H__

#define MIN_ANGLE 90
#define MAX_ANGLE 140
#define DELTA_ANGLE 2
#define SIZE ((MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1)

#define MAXBUF 1000

#define OFFSET 30

#define MIN_STAIRCASE_HEIGHT 100

#define MIN_FRONT_DISTANCE 200
#define MIN_SIDE_DISTANCE 100
#define INF 100000000

#define ROAD_THRESH_LARGE 40
#define ROAD_THRESH_SMALL 30

#define LASER_CALIBRATION_NUM 5

#define TRAFFIC_THRESH 1.4

//#define MAX_Q1_SIZE 3
//#define MAX_Q2_SIZE 3

#define MAX_SPEECH_TYPE 10

#define POI_RADIUS 30
#define POI_SEARCH_RADIUS 40
#define POI_TYPE "060200|060400|070400|090100|150200|150400|150500|150600|150700|160100|200300|190302"
#define POI_SAME_WAIT_SEC 20
#define POI_SAME_TRANSPORT_SEC 10

void submit(const char*, const char*);

#define PI acos(-1.0)

#endif

