#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "hwmon.h"

char *hwmon_resolve_path(const char *path)
{
    const char *base;
    DIR *dir;
    struct dirent *entry;
    char *resolved = NULL;

    /* Only attempt resolution when the last path component is exactly "hwmon" */
    base = strrchr(path, '/');
    if (!base || strcmp(base + 1, "hwmon") != 0)
        return strdup(path);

    dir = opendir(path);
    if (!dir)
        return strdup(path);

    while ((entry = readdir(dir)) != NULL) {
        /* Match hwmon followed by one or more digits, e.g. hwmon0, hwmon12 */
        if (strncmp(entry->d_name, "hwmon", 5) != 0)
            continue;
        if (entry->d_name[5] < '0' || entry->d_name[5] > '9')
            continue;

        {
            size_t needed = strlen(path) + 1 + strlen(entry->d_name) + 1;
            resolved = malloc(needed);
            if (resolved)
                snprintf(resolved, needed, "%s/%s", path, entry->d_name);
        }
        break;
    }

    closedir(dir);

    if (!resolved)
        return strdup(path);

    DBG("hwmon: resolved %s -> %s\n", path, resolved);
    return resolved;
}
