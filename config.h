
#include "common.h"

struct fand_config {
    struct zone *zones[MAX_ZONES];
    int zones_len;
};

struct fand_config *fand_config_load(const char *cfg_path);
void fand_config_enable(struct fand_config *cfg);
void fand_config_disable(struct fand_config *cfg);
void fand_config_destroy(struct fand_config *cfg);
