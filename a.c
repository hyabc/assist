#include <stdio.h>
#include <stdlib.h>
#include <gps.h>
#include <unistd.h>
#include <math.h>
int main() {
int rc;
struct gps_data_t gps_data;
gps_open("localhost", "2947", &gps_data);
gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

    if (gps_waiting (&gps_data, 2000000)) {
        /* read data */
        if (gps_read(&gps_data) == -1) {
            printf("error occured reading gps data. code: %d, reason: %s\n", rc, gps_errstr(rc));
        } else {
            /* Display data from the GPS receiver. */
            if (gps_data.set) {
                    printf("latitude: %f, longitude: %f, speed: %f, timestamp: %lf\n", gps_data.fix.latitude, gps_data.fix.longitude, gps_data.fix.speed, gps_data.fix.time); //EDIT: Replaced tv.tv_sec with gps_data.fix.time
            }
        }
    }


gps_stream(&gps_data, WATCH_DISABLE, NULL);
gps_close (&gps_data);

return EXIT_SUCCESS;
}
