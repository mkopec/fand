#include <string.h>
#include <stdio.h>

#include "fan.h"
#include "sensor.h"
#include "curve.h"
#include "zone.h"
#include "config.h"

int main()
{
    struct config *cfg = fand_config_create("fand.conf");

    fand_config_destroy(cfg);

    return 0;
}

