
#include "common.h"

struct config {
    struct zone *zones[MAX_ZONES];
    int zones_len;
};

struct config *fand_config_create(char *cfg_path);
void fand_config_destroy(struct config *cfg);
