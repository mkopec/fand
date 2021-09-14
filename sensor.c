#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "sensor.h"

float sensor_poll (struct sensor *s)
{
    int val;
    FILE *fd;

    fd = fopen(s->temp_path, "r");

    if (fd == NULL)
        return -1;

    fscanf (fd, "%d", &val);

    fclose (fd);
    return ((float)val / 1000) + s->offset;
}

struct sensor *sensor_create (const char *hwmon_path, int index, int offset)
{
    struct sensor *s = malloc(sizeof(struct sensor));

    if (!s)
        return NULL;

    s->hwmon_path = strdup(hwmon_path);
    s->index = index;
    s->offset = offset;

    s->temp_path = malloc(MAX_PATH * sizeof(char));

    sprintf (s->temp_path, "%s/temp%d_input", hwmon_path, index);

    return s;
}

void sensor_destroy (struct sensor *s)
{
    free(s->hwmon_path);
    free(s->temp_path);
    free(s);
}
