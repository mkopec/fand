#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "hwmon.h"
#include "fan.h"
#include "curve.h"

static int fan_set_duty_cycle(struct fan *f, int duty_cycle)
{
    FILE *fd;

    if (f->hysteresis > 0 && f->last_pwm >= 0 &&
        abs(duty_cycle - f->last_pwm) <= f->hysteresis)
        return 0;

    fd = fopen(f->pwm_path, "w");

    if (fd == NULL) {
        DBG("fan: failed to open %s\n", f->pwm_path);
        return -1;
    }

    if (fprintf(fd, "%d\n", duty_cycle) < 0) {
        fclose(fd);
        return -1;
    }

    fclose(fd);
    f->last_pwm = duty_cycle;
    return 0;
}

int fan_update(struct fan *f, float sensor_val)
{
    return fan_set_duty_cycle(f, curve_get_value(f->curve, sensor_val));
}

static int fan_set_mode(struct fan *f, int mode)
{
    FILE *fd;

    fd = fopen(f->pwm_enable_path, "w");

    if (fd == NULL)
        return -1;

    if (fprintf(fd, "%d\n", mode) < 0) {
        fclose(fd);
        return -1;
    }

    fclose(fd);
    return 0;
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

    f->hwmon_path = hwmon_resolve_path(hwmon_path);
    if (!f->hwmon_path) {
        free(f);
        return NULL;
    }
    f->index = index;
    f->curve = c;
    f->hysteresis = 0;
    f->last_pwm = -1;

    f->pwm_path = malloc(MAX_PATH * sizeof(char));
    f->rpm_path = malloc(MAX_PATH * sizeof(char));
    f->pwm_enable_path = malloc(MAX_PATH * sizeof(char));

    snprintf(f->pwm_path, MAX_PATH, "%s/pwm%d", f->hwmon_path, index);
    snprintf(f->rpm_path, MAX_PATH, "%s/fan%d_input", f->hwmon_path, index);
    snprintf(f->pwm_enable_path, MAX_PATH, "%s/pwm%d_enable", f->hwmon_path, index);

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
