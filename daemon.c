#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "zone.h"
#include "config.h"
#include "sensor.h"
#include "fan.h"

static volatile sig_atomic_t g_quit = 0;
static volatile sig_atomic_t g_reload = 0;

static void handle_quit(int sig) { (void)sig; g_quit = 1; }
static void handle_reload(int sig) { (void)sig; g_reload = 1; }

static void setup_signals(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = handle_quit;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);

    sa.sa_handler = handle_reload;
    sigaction(SIGHUP, &sa, NULL);
}

static int validate_config(struct fand_config *cfg)
{
    int i, j;

    for (i = 0; i < cfg->zones_len; ++i) {
        for (j = 0; j < cfg->zones[i]->sensors_len; ++j) {
            if (access(cfg->zones[i]->sensors[j]->temp_path, R_OK) < 0) {
                DBG("daemon: sensor path not accessible: %s\n",
                    cfg->zones[i]->sensors[j]->temp_path);
                return -1;
            }
        }
        for (j = 0; j < cfg->zones[i]->fans_len; ++j) {
            if (access(cfg->zones[i]->fans[j]->pwm_path, W_OK) < 0) {
                DBG("daemon: fan path not accessible: %s\n",
                    cfg->zones[i]->fans[j]->pwm_path);
                return -1;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *cfg_path = argc > 1 ? argv[1] : "fand.conf";
    int i;

    DBG("fand %s starting\n", FAND_VERSION);

    setup_signals();

    struct fand_config *cfg = fand_config_load(cfg_path);

    if (cfg == NULL) {
        DBG("daemon: failed to load config, exiting\n");
        return EXIT_FAILURE;
    }

    if (validate_config(cfg) < 0) {
        DBG("daemon: config validation failed, exiting\n");
        fand_config_destroy(cfg);
        return EXIT_FAILURE;
    }

    fand_config_enable(cfg);

    while (!g_quit) {
        if (g_reload) {
            g_reload = 0;
            struct fand_config *new_cfg = fand_config_load(cfg_path);
            if (new_cfg == NULL) {
                DBG("daemon: failed to reload config, keeping current\n");
            } else {
                fand_config_disable(cfg);
                fand_config_destroy(cfg);
                cfg = new_cfg;
                fand_config_enable(cfg);
                DBG("daemon: config reloaded\n");
            }
        }

        for (i = 0; i < cfg->zones_len; ++i) {
            if (zone_update(cfg->zones[i]) < 0)
                DBG("daemon: zone %d update failed\n", i);
        }

        sleep(cfg->poll_interval);
    }

    DBG("daemon: shutting down\n");
    fand_config_disable(cfg);
    fand_config_destroy(cfg);

    return EXIT_SUCCESS;
}
