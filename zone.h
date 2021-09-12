#include "common.h"

struct zone {
    struct  sensor *sensors[MAX_ZONE_SIZE];
    struct  fan *fans[MAX_ZONE_SIZE];
    int     sensors_len;
    int     fans_len;
};

int zone_update(struct zone *z);
int zone_attach_fan(struct zone *z, struct fan *f);
int zone_attach_sensor(struct zone *z, struct sensor *s);
struct zone *zone_create();
void zone_destroy(struct zone *z);