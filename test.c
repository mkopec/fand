#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "curve.h"
#include "fan.h"
#include "sensor.h"
#include "zone.h"
#include "config.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond) do { \
    if (cond) { \
        tests_passed++; \
    } else { \
        fprintf(stderr, "FAIL: %s:%d: assertion failed: %s\n", __FILE__, __LINE__, #cond); \
        tests_failed++; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    int _a = (a); int _b = (b); \
    if (_a == _b) { \
        tests_passed++; \
    } else { \
        fprintf(stderr, "FAIL: %s:%d: expected %d, got %d\n", __FILE__, __LINE__, _b, _a); \
        tests_failed++; \
    } \
} while(0)

/* curve tests */

static void test_curve_create_destroy(void)
{
    int inputs[]  = {30, 40, 50};
    int outputs[] = {100, 200, 255};
    struct curve *c = curve_create(inputs, outputs, 3);

    ASSERT(c != NULL);
    ASSERT_EQ(c->length, 3);
    ASSERT_EQ(c->inputs[0], 30);
    ASSERT_EQ(c->outputs[0], 100);

    curve_destroy(c);
    tests_passed++; /* destroy didn't crash */
}

static void test_curve_below_min(void)
{
    int inputs[]  = {30, 40, 50};
    int outputs[] = {100, 200, 255};
    struct curve *c = curve_create(inputs, outputs, 3);

    /* below minimum temperature: should return minimum speed */
    ASSERT_EQ(curve_get_value(c, 10.0f), 100);
    ASSERT_EQ(curve_get_value(c, 29.9f), 100);
    ASSERT_EQ(curve_get_value(c, 30.0f), 100);

    curve_destroy(c);
}

static void test_curve_above_max(void)
{
    int inputs[]  = {30, 40, 50};
    int outputs[] = {100, 200, 255};
    struct curve *c = curve_create(inputs, outputs, 3);

    /* above maximum temperature: should return maximum speed */
    ASSERT_EQ(curve_get_value(c, 50.0f), 255);
    ASSERT_EQ(curve_get_value(c, 60.0f), 255);
    ASSERT_EQ(curve_get_value(c, 100.0f), 255);

    curve_destroy(c);
}

static void test_curve_exact_point(void)
{
    int inputs[]  = {30, 40, 50};
    int outputs[] = {100, 200, 255};
    struct curve *c = curve_create(inputs, outputs, 3);

    /* exact curve points should return the corresponding output */
    ASSERT_EQ(curve_get_value(c, 40.0f), 200);

    curve_destroy(c);
}

static void test_curve_interpolation(void)
{
    int inputs[]  = {30, 40, 50};
    int outputs[] = {100, 200, 255};
    struct curve *c = curve_create(inputs, outputs, 3);

    /*
     * Between 30 and 40: slope = (200-100)/(40-30) = 10
     * at 35: 10*(35-30)+100 = 150
     */
    ASSERT_EQ(curve_get_value(c, 35.0f), 150);

    /*
     * Between 40 and 50: slope = (255-200)/(50-40) = 5.5
     * at 45: 5.5*(45-40)+200 = 227 (truncated to int)
     */
    ASSERT_EQ(curve_get_value(c, 45.0f), 227);

    curve_destroy(c);
}

static void test_curve_single_point(void)
{
    int inputs[]  = {50};
    int outputs[] = {128};
    struct curve *c = curve_create(inputs, outputs, 1);

    /* any value should return the single output */
    ASSERT_EQ(curve_get_value(c, 0.0f),   128);
    ASSERT_EQ(curve_get_value(c, 50.0f),  128);
    ASSERT_EQ(curve_get_value(c, 100.0f), 128);

    curve_destroy(c);
}

/* sensor tests */

static void test_sensor_create_destroy(void)
{
    struct sensor *s = sensor_create("/sys/class/hwmon/hwmon0", 1, 0);

    ASSERT(s != NULL);
    ASSERT(s->hwmon_path != NULL);
    ASSERT_EQ(s->index, 1);
    ASSERT_EQ(s->offset, 0);
    ASSERT(strstr(s->temp_path, "temp1_input") != NULL);

    sensor_destroy(s);
    tests_passed++; /* destroy didn't crash */
}

static void test_sensor_create_with_offset(void)
{
    struct sensor *s = sensor_create("/sys/class/hwmon/hwmon2", 3, 5);

    ASSERT(s != NULL);
    ASSERT_EQ(s->index, 3);
    ASSERT_EQ(s->offset, 5);
    ASSERT(strstr(s->temp_path, "temp3_input") != NULL);

    sensor_destroy(s);
}

/* fan tests */

static void test_fan_create_destroy(void)
{
    int inputs[]  = {30, 40};
    int outputs[] = {100, 255};
    struct curve *c = curve_create(inputs, outputs, 2);
    struct fan *f = fan_create("/sys/class/hwmon/hwmon0", 1, c);

    ASSERT(f != NULL);
    ASSERT(f->hwmon_path != NULL);
    ASSERT_EQ(f->index, 1);
    ASSERT(f->curve != NULL);
    ASSERT(strstr(f->pwm_path, "pwm1") != NULL);
    ASSERT(strstr(f->pwm_enable_path, "pwm1_enable") != NULL);
    ASSERT(strstr(f->rpm_path, "fan1_input") != NULL);

    fan_destroy(f);
    tests_passed++; /* destroy didn't crash, including curve */
}

/* zone tests */

static void test_zone_create_destroy(void)
{
    struct zone *z = zone_create();

    ASSERT(z != NULL);
    ASSERT_EQ(z->fans_len, 0);
    ASSERT_EQ(z->sensors_len, 0);

    zone_destroy(z);
    tests_passed++; /* destroy didn't crash */
}

static void test_zone_attach_sensor(void)
{
    struct zone *z = zone_create();
    struct sensor *s1 = sensor_create("/sys/class/hwmon/hwmon0", 1, 0);
    struct sensor *s2 = sensor_create("/sys/class/hwmon/hwmon0", 2, 0);
    int ret;

    ret = zone_attach_sensor(z, s1);
    ASSERT_EQ(ret, 1);
    ASSERT_EQ(z->sensors_len, 1);

    ret = zone_attach_sensor(z, s2);
    ASSERT_EQ(ret, 2);
    ASSERT_EQ(z->sensors_len, 2);

    zone_destroy(z); /* also destroys sensors */
}

static void test_zone_attach_fan(void)
{
    struct zone *z = zone_create();
    int inputs[] = {30, 50};
    int outputs[] = {100, 255};
    struct fan *f1 = fan_create("/sys/class/hwmon/hwmon0", 1, curve_create(inputs, outputs, 2));
    struct fan *f2 = fan_create("/sys/class/hwmon/hwmon0", 2, curve_create(inputs, outputs, 2));
    int ret;

    ret = zone_attach_fan(z, f1);
    ASSERT_EQ(ret, 1);
    ASSERT_EQ(z->fans_len, 1);

    ret = zone_attach_fan(z, f2);
    ASSERT_EQ(ret, 2);
    ASSERT_EQ(z->fans_len, 2);

    zone_destroy(z); /* also destroys fans */
}

static void test_zone_attach_limit(void)
{
    struct zone *z = zone_create();
    struct sensor *extra;
    int i, ret;

    /* fill up to MAX_ZONE_SIZE */
    for (i = 0; i < MAX_ZONE_SIZE; i++) {
        struct sensor *s = sensor_create("/sys/class/hwmon/hwmon0", i + 1, 0);
        ret = zone_attach_sensor(z, s);
        ASSERT(ret > 0);
    }
    ASSERT_EQ(z->sensors_len, MAX_ZONE_SIZE);

    /* next attach should fail */
    extra = sensor_create("/sys/class/hwmon/hwmon0", MAX_ZONE_SIZE + 1, 0);
    ret = zone_attach_sensor(z, extra);
    ASSERT_EQ(ret, -1);
    ASSERT_EQ(z->sensors_len, MAX_ZONE_SIZE);

    sensor_destroy(extra);
    zone_destroy(z);
}

/* config tests */

static void test_config_load_valid(void)
{
    const char *cfg_path = "/tmp/fand_test.conf";
    FILE *f = fopen(cfg_path, "w");
    ASSERT(f != NULL);

    fprintf(f,
        "zones:\n"
        "  - sensors:\n"
        "      - path: /sys/class/hwmon/hwmon0\n"
        "        index: 1\n"
        "        offset: 0\n"
        "    fans:\n"
        "      - path: /sys/class/hwmon/hwmon0\n"
        "        index: 1\n"
        "        curve:\n"
        "          temperatures: [30, 40, 50]\n"
        "          speeds: [100, 200, 255]\n"
    );
    fclose(f);

    struct fand_config *cfg = fand_config_load(cfg_path);

    ASSERT(cfg != NULL);
    ASSERT_EQ(cfg->zones_len, 1);
    ASSERT_EQ(cfg->zones[0]->sensors_len, 1);
    ASSERT_EQ(cfg->zones[0]->fans_len, 1);

    fand_config_destroy(cfg);
    remove(cfg_path);
}

static void test_config_load_missing_file(void)
{
    struct fand_config *cfg = fand_config_load("/tmp/fand_nonexistent_12345.conf");
    ASSERT(cfg == NULL);
}

static void test_config_load_no_zones(void)
{
    const char *cfg_path = "/tmp/fand_test_empty.conf";
    FILE *f = fopen(cfg_path, "w");
    ASSERT(f != NULL);

    fprintf(f, "zones: []\n");
    fclose(f);

    struct fand_config *cfg = fand_config_load(cfg_path);
    ASSERT(cfg == NULL);

    remove(cfg_path);
}

static void test_config_load_multiple_zones(void)
{
    const char *cfg_path = "/tmp/fand_test_multi.conf";
    FILE *f = fopen(cfg_path, "w");
    ASSERT(f != NULL);

    fprintf(f,
        "zones:\n"
        "  - sensors:\n"
        "      - path: /sys/class/hwmon/hwmon0\n"
        "        index: 1\n"
        "        offset: 0\n"
        "    fans:\n"
        "      - path: /sys/class/hwmon/hwmon0\n"
        "        index: 1\n"
        "        curve:\n"
        "          temperatures: [30, 50]\n"
        "          speeds: [100, 255]\n"
        "  - sensors:\n"
        "      - path: /sys/class/hwmon/hwmon1\n"
        "        index: 1\n"
        "        offset: 0\n"
        "    fans:\n"
        "      - path: /sys/class/hwmon/hwmon1\n"
        "        index: 2\n"
        "        curve:\n"
        "          temperatures: [25, 45]\n"
        "          speeds: [80, 255]\n"
    );
    fclose(f);

    struct fand_config *cfg = fand_config_load(cfg_path);

    ASSERT(cfg != NULL);
    ASSERT_EQ(cfg->zones_len, 2);

    fand_config_destroy(cfg);
    remove(cfg_path);
}

int main(void)
{
    printf("Running fand tests...\n");

    test_curve_create_destroy();
    test_curve_below_min();
    test_curve_above_max();
    test_curve_exact_point();
    test_curve_interpolation();
    test_curve_single_point();

    test_sensor_create_destroy();
    test_sensor_create_with_offset();

    test_fan_create_destroy();

    test_zone_create_destroy();
    test_zone_attach_sensor();
    test_zone_attach_fan();
    test_zone_attach_limit();

    test_config_load_valid();
    test_config_load_missing_file();
    test_config_load_no_zones();
    test_config_load_multiple_zones();

    printf("\n%d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
