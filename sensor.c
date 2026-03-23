#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "hwmon.h"
#include "sensor.h"

float sensor_poll (struct sensor *s)
{
    int val;
    FILE *fd;

    fd = fopen(s->temp_path, "r");

    if (fd == NULL) {
        DBG("sensor: failed to open %s\n", s->temp_path);
        return NAN;
    }

    if (fscanf(fd, "%d", &val) != 1) {
        DBG("sensor: failed to read value from %s\n", s->temp_path);
        fclose(fd);
        return NAN;
    }

    fclose(fd);
    return ((float)val / 1000) + s->offset;
}

struct sensor *sensor_create (const char *hwmon_path, int index, int offset)
{
    struct sensor *s = malloc(sizeof(struct sensor));

    if (!s)
        return NULL;

    s->hwmon_path = hwmon_resolve_path(hwmon_path);
    if (!s->hwmon_path) {
        free(s);
        return NULL;
    }
    s->index = index;
    s->offset = offset;

    s->temp_path = malloc(MAX_PATH * sizeof(char));

    snprintf(s->temp_path, MAX_PATH, "%s/temp%d_input", s->hwmon_path, index);

    return s;
}

void sensor_destroy (struct sensor *s)
{
    free(s->hwmon_path);
    free(s->temp_path);
    free(s);
}
