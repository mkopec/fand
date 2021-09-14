#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "zone.h"
#include "config.h"

int main()
{
	bool quit = false;
	int state = 0;

	DBG("fand %s starting\n", FAND_VERSION);

	struct fand_config *cfg = fand_config_load("fand.conf");

	if (cfg == NULL) {
		DBG("daemon: failed to load config, exiting\n");
		return EXIT_FAILURE;
	}

	fand_config_enable(cfg);

	while (!quit)
	{
		for (int i = 0; i < cfg->zones_len; ++i)
		{
			state = zone_update(cfg->zones[i]);

			if (state == 0)
				quit = true;
		}

		sleep(1);
	}

	fand_config_disable(cfg);
	fand_config_destroy(cfg);

	return EXIT_SUCCESS;
}
