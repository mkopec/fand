#include <stdlib.h>

#include "common.h"
#include "zone.h"
#include "fan.h"
#include "sensor.h"

static float max_sensor_val(struct zone *z)
{
    int i;
    float sensor_val;
    float max_sensor_val = 0;

    for (i = 0; i < z->sensors_len; ++i) {
        sensor_val = sensor_poll(z->sensors[i]);
        if (sensor_val > max_sensor_val)
            max_sensor_val = sensor_val;
    }

    return max_sensor_val;
}

int zone_attach_fan(struct zone *z, struct fan *f)
{
    if (z->fans_len >= MAX_ZONE_SIZE)
        return -1;

    z->fans[z->fans_len] = f;

    z->fans_len++;

    return z->fans_len;
}

int zone_attach_sensor(struct zone *z, struct sensor *s)
{
    if (z->sensors_len >= MAX_ZONE_SIZE)
        return -1;

    z->sensors[z->sensors_len] = s;

    z->sensors_len++;

    return z->sensors_len;
}

int zone_update(struct zone *z)
{
    int i, status, rv;
    float sensor_val;

    rv = 0;
    sensor_val = max_sensor_val(z);

    for (i = 0; i < z->fans_len; ++i) {
        status = fan_update(z->fans[i], sensor_val);
        if (status)
            rv = status;
    }

    return rv;
}

struct zone *zone_create()
{
    struct zone *z = malloc(sizeof(struct zone));

    z->sensors_len = 0;
    z->fans_len = 0;

    return z;
}

void zone_destroy(struct zone *z)
{
    int i;
    for (i = 0; i < z->sensors_len; ++i)
        sensor_destroy(z->sensors[i]);
    for (i = 0; i < z->fans_len; ++i)
        fan_destroy(z->fans[i]);
    free(z);
}
