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
	
	struct config *cfg = fand_config_load("fand.conf");

	if (!cfg)
		return EXIT_FAILURE;


	while (!quit)
	{
		for (int i = 0; i < cfg->zones_len; ++i)
		{
			state = zone_update(cfg->zones[i]);

			if (state != 0)
				quit = true;
		}

		sleep(1);
	}

	return EXIT_SUCCESS;
}
