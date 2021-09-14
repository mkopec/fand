#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

#include "fan.h"
#include "zone.h"
#include "sensor.h"
#include "curve.h"
#include "config.h"
#include "common.h"

static int config_zone_attach_sensors(struct zone *z, config_setting_t *zone)
{
    config_setting_t *sensors;

    sensors = config_setting_lookup(zone, "sensors");

    if (sensors != NULL) {
        int count = config_setting_length(sensors);
        int i;
        for (i = 0; i < count; ++i) {
            int index, offset;
            const char *path;
            config_setting_t *sensor = config_setting_get_elem(sensors, i);

            if (sensor == NULL)
                continue;

            if (!config_setting_lookup_string(sensor, "path", &path)) {
                DBG("config: skipping sensor with no path\n");
                continue;
            }

            if (!config_setting_lookup_int(sensor, "index", &index)) {
                DBG("config: sensor at %s has no index, skipping\n", path);
                continue;
            }

            if (!config_setting_lookup_int(sensor, "offset", &offset)) {
                offset = 0;
            }

            zone_attach_sensor(z, sensor_create(path, index, offset));
            DBG("Adding sensor at %s, index %d\n", path, index);
        }
    }
    return 0;
}

static int config_zone_attach_fans(struct zone *z, config_setting_t *zone)
{
    config_setting_t *fans;

    fans = config_setting_lookup(zone, "fans");

    if (fans != NULL) {
        int count = config_setting_length(fans);
        int i;
        for (i = 0; i < count; ++i) {
            int index;
            const char *path;
            int *t, *p;
            int count;
            config_setting_t *fan = config_setting_get_elem(fans, i);

            if (fan == NULL)
                continue;

            if (!config_setting_lookup_string(fan, "path", &path)) {
                DBG("config: skipping fan with no path\n");
                continue;
            }

            if (!config_setting_lookup_int(fan, "index", &index)) {
                DBG("config: fan at %s has no index, skipping\n", path);
                continue;
            }

            config_setting_t *curve = config_setting_get_member(fan, "curve");
            if (curve == NULL) {
                DBG("config: fan at %s index %d has no curve, skipping\n",
                    path, index);
                continue;
            }

            config_setting_t *temperatures = config_setting_get_member(curve, "temperatures");
            if (temperatures == NULL) {
                DBG("config: fan at %s index %d has no temperatures, skipping\n",
                    path, index);
                continue;
            }

            config_setting_t *speeds = config_setting_get_member(curve, "speeds");
            if (speeds == NULL) {
                DBG("config: fan at %s index %d has no speeds, skipping\n",
                    path, index);
                continue;
            }

            count = config_setting_length(temperatures);
            if (config_setting_length(speeds) != count) {
                DBG("config: fan at %s index %d has wrongly formatted curve, skipping\n",
                    path, index);
                continue;
            }

            t = malloc(count * sizeof(int));
            for (int j = 0; j < count; ++j) {
                t[j] = config_setting_get_int_elem(temperatures, j);
            }

            p = malloc(count * sizeof(int));
            for (int j = 0; j < count; ++j) {
                p[j] = config_setting_get_int_elem(speeds, j);
            }

            zone_attach_fan(z, fan_create(path, index, curve_create(t, p, count)));
            DBG("Adding fan at %s, index %d\n", path, index);

            free (t);
            free (p);
        }
    }
    return 0;
}

struct fand_config *fand_config_load(const char *cfg_path)
{
    struct fand_config *cfg = NULL;
    int i;
    config_t ct;
    config_setting_t *setting;

    config_init(&ct);

    if(!config_read_file(&ct, cfg_path)) {
        DBG("config: failed to open config file %s\n", cfg_path);
        goto cleanup;
    }

    setting = config_lookup(&ct, "zones");

    if (setting == NULL) {
        DBG("config: no zones are defined\n");
        goto cleanup;
    }

    cfg = malloc(sizeof(struct fand_config));
    cfg->zones_len = config_setting_length(setting);

    for (i = 0; i < cfg->zones_len; ++i) {
        config_setting_t *zone = config_setting_get_elem(setting, i);
        cfg->zones[i] = zone_create();
        config_zone_attach_sensors(cfg->zones[i], zone);
        config_zone_attach_fans(cfg->zones[i], zone);
    }

    DBG("configuration file read, found %d zones\n", cfg->zones_len);

cleanup:
    config_destroy(&ct);
    return cfg;
}

void fand_config_enable(struct fand_config *cfg)
{
    int i, j;
    for (i = 0; i < cfg->zones_len; ++i)
        for (j = 0; j < cfg->zones[i]->fans_len; ++j)
            fan_enable(cfg->zones[i]->fans[j]);
}

void fand_config_disable(struct fand_config *cfg)
{
    int i, j;
    for (i = 0; i < cfg->zones_len; ++i)
        for (j = 0; j < cfg->zones[i]->fans_len; ++j)
            fan_disable(cfg->zones[i]->fans[j]);
}

void fand_config_destroy(struct fand_config *cfg)
{
    int i;
    for (i = 0; i < cfg->zones_len; ++i)
        zone_destroy(cfg->zones[i]);
    free(cfg);
}

