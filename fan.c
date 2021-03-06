#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "fan.h"
#include "curve.h"

static int fan_set_duty_cycle(struct fan *f, int duty_cycle)
{
    FILE *fd;
    int rv = 0;

    fd = fopen(f->pwm_path, "w");

    if (fd == NULL)
        return -1;

    rv = fprintf (fd, "%d\n", duty_cycle);

    fclose (fd);
    return rv;
}

int fan_update(struct fan *f, float sensor_val)
{
    return fan_set_duty_cycle(f, curve_get_value(f->curve, sensor_val));
}

static int fan_set_mode(struct fan *f, int mode)
{
    FILE *fd;
    int rv = 0;

    fd = fopen(f->pwm_enable_path, "w");

    if (fd == NULL)
        return -1;

    rv = fprintf (fd, "%d\n", mode);

    fclose (fd);
    return rv;
}

int fan_enable(struct fan *f)
{
    DBG("Enabling manual PWM control for fan %s index %d\n", f->hwmon_path, f->index);
    return fan_set_mode(f, 1);
}

int fan_disable(struct fan *f)
{
    DBG("Restoring automatic control for fan %s index %d\n", f->hwmon_path, f->index);
    return fan_set_mode(f, 5);
}

struct fan *fan_create (const char *hwmon_path, int index, struct curve *c)
{
    struct fan *f = malloc(sizeof(struct fan));

    if (!f)
        return NULL;

    f->hwmon_path = strdup(hwmon_path);
    f->index = index;
    f->curve = c;

    f->pwm_path = malloc(MAX_PATH * sizeof(char));
    f->rpm_path = malloc(MAX_PATH * sizeof(char));
    f->pwm_enable_path = malloc(MAX_PATH * sizeof(char));

    sprintf (f->pwm_path, "%s/pwm%d", hwmon_path, index);
    sprintf (f->rpm_path, "%s/fan%d_input", hwmon_path, index);
    sprintf (f->pwm_enable_path, "%s/pwm%d_enable", hwmon_path, index);

    return f;
}

void fan_destroy (struct fan *f)
{
    free(f->hwmon_path);
    free(f->pwm_path);
    free(f->rpm_path);
    free(f->pwm_enable_path);
    curve_destroy(f->curve);
    free(f);
}
