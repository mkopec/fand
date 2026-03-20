#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

#include "fan.h"
#include "zone.h"
#include "sensor.h"
#include "curve.h"
#include "config.h"
#include "common.h"

static int parse_int_sequence(yaml_parser_t *parser, int **out_arr, int *out_count)
{
    yaml_event_t event;
    int *arr = NULL;
    int count = 0;
    int capacity = 8;

    arr = malloc(capacity * sizeof(int));
    if (!arr)
        return 0;

    while (1) {
        if (!yaml_parser_parse(parser, &event)) {
            free(arr);
            return 0;
        }

        if (event.type == YAML_SEQUENCE_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            if (count >= capacity) {
                capacity *= 2;
                int *tmp = realloc(arr, capacity * sizeof(int));
                if (!tmp) {
                    yaml_event_delete(&event);
                    free(arr);
                    return 0;
                }
                arr = tmp;
            }
            arr[count++] = atoi((char *)event.data.scalar.value);
        }
        yaml_event_delete(&event);
    }

    *out_arr = arr;
    *out_count = count;
    return 1;
}

static int skip_node(yaml_parser_t *parser, yaml_event_type_t start_type)
{
    yaml_event_t event;
    int depth = 1;

    while (depth > 0) {
        if (!yaml_parser_parse(parser, &event))
            return 0;
        if (event.type == YAML_SEQUENCE_START_EVENT || event.type == YAML_MAPPING_START_EVENT)
            depth++;
        else if (event.type == YAML_SEQUENCE_END_EVENT || event.type == YAML_MAPPING_END_EVENT)
            depth--;
        yaml_event_delete(&event);
    }
    return 1;
}

static int parse_curve(yaml_parser_t *parser, int **out_temps, int **out_speeds, int *out_count)
{
    yaml_event_t event;
    int *temps = NULL, *speeds = NULL;
    int t_count = 0, s_count = 0;

    /* Consume MAPPING_START */
    if (!yaml_parser_parse(parser, &event))
        return 0;
    if (event.type != YAML_MAPPING_START_EVENT) {
        yaml_event_delete(&event);
        return 0;
    }
    yaml_event_delete(&event);

    while (1) {
        if (!yaml_parser_parse(parser, &event)) {
            free(temps);
            free(speeds);
            return 0;
        }

        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            char key[64];
            strncpy(key, (char *)event.data.scalar.value, sizeof(key) - 1);
            key[sizeof(key) - 1] = '\0';
            yaml_event_delete(&event);

            if (!yaml_parser_parse(parser, &event)) {
                free(temps);
                free(speeds);
                return 0;
            }

            if (event.type == YAML_SEQUENCE_START_EVENT) {
                yaml_event_delete(&event);
                if (strcmp(key, "temperatures") == 0) {
                    if (!parse_int_sequence(parser, &temps, &t_count)) {
                        free(speeds);
                        return 0;
                    }
                } else if (strcmp(key, "speeds") == 0) {
                    if (!parse_int_sequence(parser, &speeds, &s_count)) {
                        free(temps);
                        return 0;
                    }
                } else {
                    if (!skip_node(parser, YAML_SEQUENCE_START_EVENT)) {
                        free(temps);
                        free(speeds);
                        return 0;
                    }
                }
            } else {
                yaml_event_delete(&event);
            }
        } else {
            yaml_event_delete(&event);
        }
    }

    if (!temps || !speeds || t_count != s_count) {
        DBG("config: curve has invalid or mismatched temperatures/speeds\n");
        free(temps);
        free(speeds);
        return 0;
    }

    *out_temps = temps;
    *out_speeds = speeds;
    *out_count = t_count;
    return 1;
}

static int parse_sensors(yaml_parser_t *parser, struct zone *z)
{
    yaml_event_t event;

    while (1) {
        if (!yaml_parser_parse(parser, &event))
            return 0;

        if (event.type == YAML_SEQUENCE_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type == YAML_MAPPING_START_EVENT) {
            yaml_event_delete(&event);
            char path[MAX_PATH] = "";
            int index = -1, offset = 0;

            while (1) {
                if (!yaml_parser_parse(parser, &event))
                    return 0;

                if (event.type == YAML_MAPPING_END_EVENT) {
                    yaml_event_delete(&event);
                    break;
                }

                if (event.type == YAML_SCALAR_EVENT) {
                    char key[64];
                    strncpy(key, (char *)event.data.scalar.value, sizeof(key) - 1);
                    key[sizeof(key) - 1] = '\0';
                    yaml_event_delete(&event);

                    if (!yaml_parser_parse(parser, &event))
                        return 0;

                    if (event.type == YAML_SCALAR_EVENT) {
                        if (strcmp(key, "path") == 0) {
                            strncpy(path, (char *)event.data.scalar.value, MAX_PATH - 1);
                        } else if (strcmp(key, "index") == 0) {
                            index = atoi((char *)event.data.scalar.value);
                        } else if (strcmp(key, "offset") == 0) {
                            offset = atoi((char *)event.data.scalar.value);
                        }
                    }
                    yaml_event_delete(&event);
                } else {
                    yaml_event_delete(&event);
                }
            }

            if (path[0] == '\0') {
                DBG("config: skipping sensor with no path\n");
            } else if (index < 0) {
                DBG("config: sensor at %s has no index, skipping\n", path);
            } else {
                zone_attach_sensor(z, sensor_create(path, index, offset));
                DBG("Adding sensor at %s, index %d\n", path, index);
            }
        } else {
            yaml_event_delete(&event);
        }
    }
    return 1;
}

static int parse_fans(yaml_parser_t *parser, struct zone *z)
{
    yaml_event_t event;

    while (1) {
        if (!yaml_parser_parse(parser, &event))
            return 0;

        if (event.type == YAML_SEQUENCE_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type == YAML_MAPPING_START_EVENT) {
            yaml_event_delete(&event);
            char path[MAX_PATH] = "";
            int index = -1;
            int *temps = NULL, *speeds = NULL;
            int curve_count = 0;

            while (1) {
                if (!yaml_parser_parse(parser, &event)) {
                    free(temps);
                    free(speeds);
                    return 0;
                }

                if (event.type == YAML_MAPPING_END_EVENT) {
                    yaml_event_delete(&event);
                    break;
                }

                if (event.type == YAML_SCALAR_EVENT) {
                    char key[64];
                    strncpy(key, (char *)event.data.scalar.value, sizeof(key) - 1);
                    key[sizeof(key) - 1] = '\0';
                    yaml_event_delete(&event);

                    if (strcmp(key, "curve") == 0) {
                        if (!parse_curve(parser, &temps, &speeds, &curve_count)) {
                            free(temps);
                            free(speeds);
                            return 0;
                        }
                    } else {
                        if (!yaml_parser_parse(parser, &event)) {
                            free(temps);
                            free(speeds);
                            return 0;
                        }
                        if (event.type == YAML_SCALAR_EVENT) {
                            if (strcmp(key, "path") == 0) {
                                strncpy(path, (char *)event.data.scalar.value, MAX_PATH - 1);
                            } else if (strcmp(key, "index") == 0) {
                                index = atoi((char *)event.data.scalar.value);
                            }
                        }
                        yaml_event_delete(&event);
                    }
                } else {
                    yaml_event_delete(&event);
                }
            }

            if (path[0] == '\0') {
                DBG("config: skipping fan with no path\n");
                free(temps);
                free(speeds);
            } else if (index < 0) {
                DBG("config: fan at %s has no index, skipping\n", path);
                free(temps);
                free(speeds);
            } else if (!temps || !speeds) {
                DBG("config: fan at %s index %d has no curve, skipping\n", path, index);
                free(temps);
                free(speeds);
            } else {
                zone_attach_fan(z, fan_create(path, index, curve_create(temps, speeds, curve_count)));
                DBG("Adding fan at %s, index %d\n", path, index);
                free(temps);
                free(speeds);
            }
        } else {
            yaml_event_delete(&event);
        }
    }
    return 1;
}

static int parse_zones(yaml_parser_t *parser, struct fand_config *cfg)
{
    yaml_event_t event;

    while (1) {
        if (!yaml_parser_parse(parser, &event))
            return 0;

        if (event.type == YAML_SEQUENCE_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type == YAML_MAPPING_START_EVENT) {
            yaml_event_delete(&event);

            if (cfg->zones_len >= MAX_ZONES) {
                DBG("config: too many zones, skipping\n");
                if (!skip_node(parser, YAML_MAPPING_START_EVENT))
                    return 0;
                continue;
            }

            struct zone *z = zone_create();

            while (1) {
                if (!yaml_parser_parse(parser, &event)) {
                    zone_destroy(z);
                    return 0;
                }

                if (event.type == YAML_MAPPING_END_EVENT) {
                    yaml_event_delete(&event);
                    break;
                }

                if (event.type == YAML_SCALAR_EVENT) {
                    char key[64];
                    strncpy(key, (char *)event.data.scalar.value, sizeof(key) - 1);
                    key[sizeof(key) - 1] = '\0';
                    yaml_event_delete(&event);

                    if (!yaml_parser_parse(parser, &event)) {
                        zone_destroy(z);
                        return 0;
                    }

                    if (event.type == YAML_SEQUENCE_START_EVENT) {
                        yaml_event_delete(&event);
                        if (strcmp(key, "sensors") == 0) {
                            parse_sensors(parser, z);
                        } else if (strcmp(key, "fans") == 0) {
                            parse_fans(parser, z);
                        } else {
                            if (!skip_node(parser, YAML_SEQUENCE_START_EVENT)) {
                                zone_destroy(z);
                                return 0;
                            }
                        }
                    } else {
                        yaml_event_delete(&event);
                    }
                } else {
                    yaml_event_delete(&event);
                }
            }

            cfg->zones[cfg->zones_len++] = z;
        } else {
            yaml_event_delete(&event);
        }
    }
    return 1;
}

struct fand_config *fand_config_load(const char *cfg_path)
{
    struct fand_config *cfg = NULL;
    yaml_parser_t parser;
    yaml_event_t event;
    FILE *f;

    f = fopen(cfg_path, "r");
    if (!f) {
        DBG("config: failed to open config file %s\n", cfg_path);
        return NULL;
    }

    if (!yaml_parser_initialize(&parser)) {
        DBG("config: failed to initialize yaml parser\n");
        fclose(f);
        return NULL;
    }

    yaml_parser_set_input_file(&parser, f);

    cfg = malloc(sizeof(struct fand_config));
    if (!cfg)
        goto cleanup;
    cfg->zones_len = 0;

    /* STREAM_START */
    if (!yaml_parser_parse(&parser, &event))
        goto free_cfg;
    yaml_event_delete(&event);

    /* DOCUMENT_START */
    if (!yaml_parser_parse(&parser, &event))
        goto free_cfg;
    yaml_event_delete(&event);

    /* root MAPPING_START */
    if (!yaml_parser_parse(&parser, &event))
        goto free_cfg;
    if (event.type != YAML_MAPPING_START_EVENT) {
        yaml_event_delete(&event);
        goto free_cfg;
    }
    yaml_event_delete(&event);

    while (1) {
        if (!yaml_parser_parse(&parser, &event))
            goto free_cfg;

        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            char key[64];
            strncpy(key, (char *)event.data.scalar.value, sizeof(key) - 1);
            key[sizeof(key) - 1] = '\0';
            yaml_event_delete(&event);

            if (!yaml_parser_parse(&parser, &event))
                goto free_cfg;

            if (strcmp(key, "zones") == 0 && event.type == YAML_SEQUENCE_START_EVENT) {
                yaml_event_delete(&event);
                parse_zones(&parser, cfg);
            } else {
                yaml_event_delete(&event);
            }
        } else {
            yaml_event_delete(&event);
        }
    }

    if (cfg->zones_len == 0) {
        DBG("config: no zones are defined\n");
        goto free_cfg;
    }

    DBG("configuration file read, found %d zones\n", cfg->zones_len);
    goto cleanup;

free_cfg:
    free(cfg);
    cfg = NULL;

cleanup:
    yaml_parser_delete(&parser);
    fclose(f);
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
