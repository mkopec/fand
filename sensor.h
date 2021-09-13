#include "common.h"

struct sensor {
    char    *hwmon_path;
    int     index;
    int     offset;
    char    *temp_path;
};

int sensor_poll (struct sensor *s);
struct sensor *sensor_create (const char *hwmon_path, int index, int offset);
void sensor_destroy (struct sensor *s);