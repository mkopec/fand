#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

#include "fan.h"
#include "zone.h"
#include "sensor.h"
#include "curve.h"
#include "common.h"

struct fand_config {
    struct zone *zones[MAX_ZONES];
    int zones_len;
};

int config_zone_attach_sensors(struct zone *z, config_setting_t *zone)
{
    config_setting_t *sensors;

    sensors = config_setting_lookup(zone, "sensors");

    if (sensors != NULL) {
        int count = config_setting_length(sensors);
        int i;
        for (i = 0; i < count; ++i) {
            int index, offset;
            char *path;
            config_setting_t *sensor = config_setting_get_elem(sensors, i);
            config_setting_lookup_string(sensor, "path", &path);
            config_setting_lookup_int(sensor, "index", &index);
            config_setting_lookup_int(sensor, "offset", &offset);
            zone_attach_sensor(z, sensor_create(path, index, offset));
        }
    }
}

int config_zone_attach_fans(struct zone *z, config_setting_t *zone)
{
    config_setting_t *fans;

    fans = config_setting_lookup(zone, "fans");

    if (fans != NULL) {
        int count = config_setting_length(fans);
        int i;
        for (i = 0; i < count; ++i) {
            int index;
            char *path;
            int *t, *p;
            int tc;
            config_setting_t *fan = config_setting_get_elem(fans, i);
            config_setting_lookup_string(fan, "path", &path);
            config_setting_lookup_int(fan, "index", &index);
            config_setting_t *curve = config_setting_get_member(fan, "curve");
            config_setting_t *temperatures = config_setting_get_member(curve, "temperatures");
            config_setting_t *pwm = config_setting_get_member(curve, "pwm");
            tc = config_setting_length(temperatures);
            t = malloc(tc * sizeof(int));
            for (int j = 0; j < tc; ++j) {
                t[j] = config_setting_get_int_elem(temperatures, j);
            }
            p = malloc(tc * sizeof(int));
            for (int j = 0; j < tc; ++j) {
                p[j] = config_setting_get_int_elem(pwm, j);
            }

            zone_attach_fan(z, fan_create(path, index, curve_create(t, p, tc)));

            free (t);
            free (p);
        }
    }
}

struct fand_config *fand_config_create(char *cfg_path)
{
    struct fand_config *cfg = malloc(sizeof(struct fand_config));
    cfg->zones_len = 0;

    config_t ct;
    config_setting_t *setting;
    const char *s;

    config_init(&ct);

    if(!config_read_file(&ct, cfg_path))
        return NULL;

    setting = config_lookup(&ct, "zones");

    if (setting != NULL) {
        int count = config_setting_length(setting);
        int i;
        for (i = 0; i < count; ++i) {
            config_setting_t *zone = config_setting_get_elem(setting, i);
            cfg->zones[i] = zone_create();
            config_zone_attach_sensors(cfg->zones[i], zone);
            config_zone_attach_fans(cfg->zones[i], zone);
        }
        cfg->zones_len = count;
    }

    config_destroy(&ct);

    return cfg;
}


void fand_config_destroy(struct fand_config *cfg)
{
    int i;
    for (i = 0; i < cfg->zones_len; ++i)
        zone_destroy(cfg->zones[i]);
    free(cfg);
}

