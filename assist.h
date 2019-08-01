#ifndef __ASSIST_H__
#define __ASSIST_H__

#define MIN_ANGLE 90
#define MAX_ANGLE 130
#define DELTA_ANGLE 2
#define SIZE ((MAX_ANGLE - MIN_ANGLE) / DELTA_ANGLE + 1)

#define MAXBUF 1000

void submit(const char*, const char*);

#endif

