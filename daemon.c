#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "zone.h"
#include "config.h"

int main()
{
	struct config *cfg = fand_config_create("fand.conf");

	if (!cfg)
		return EXIT_FAILURE;

	bool quit = false;

	while (!quit)
	{
		for (int i = 0; i < cfg->zones_len; ++i)
		{
			int state = zone_update(cfg->zones[i]);
		}

		sleep(1);
	}

	return EXIT_SUCCESS;
}
