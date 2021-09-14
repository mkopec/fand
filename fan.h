#include "common.h"

struct fan {
    char    *hwmon_path;
    int     index;
    char    *pwm_path;
    char    *rpm_path;
    char    *pwm_enable_path;
    struct curve *curve;
};

int fan_update(struct fan *f, float sensor_val);
struct fan *fan_create (const char *hwmon_path, int index, struct curve *c);
int fan_enable(struct fan *f);
int fan_disable(struct fan *f);
void fan_destroy (struct fan *f);