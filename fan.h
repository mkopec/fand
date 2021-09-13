#include "common.h"

struct fan {
    char    *hwmon_path;
    int     index;
    char    *pwm_path;
    char    *rpm_path;
    struct curve *curve;
};

int fan_update(struct fan *f, int sensor_val);
struct fan *fan_create (const char *hwmon_path, int index, struct curve *c);
void fan_destroy (struct fan *f);