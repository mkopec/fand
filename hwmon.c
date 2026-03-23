#include <dirent.h>
#include <glob.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "hwmon.h"

char *hwmon_resolve_path(const char *path)
{
    const char *base;
    const char *work;
    char *glob_result = NULL;
    DIR *dir;
    struct dirent *entry;
    char *resolved = NULL;

    /* Expand wildcards if present */
    if (strchr(path, '*') || strchr(path, '?')) {
        glob_t g;
        if (glob(path, GLOB_NOSORT, NULL, &g) == 0 && g.gl_pathc > 0)
            glob_result = strdup(g.gl_pathv[0]);
        globfree(&g);
        if (!glob_result) {
            DBG("hwmon: no match for glob pattern %s\n", path);
            return strdup(path);
        }
        DBG("hwmon: glob %s -> %s\n", path, glob_result);
        work = glob_result;
    } else {
        work = path;
    }

    /* Only attempt hwmon resolution when the last path component is exactly "hwmon" */
    base = strrchr(work, '/');
    if (!base || strcmp(base + 1, "hwmon") != 0)
        return glob_result ? glob_result : strdup(work);

    dir = opendir(work);
    if (!dir)
        return glob_result ? glob_result : strdup(work);

    while ((entry = readdir(dir)) != NULL) {
        /* Match hwmon followed by one or more digits, e.g. hwmon0, hwmon12 */
        if (strncmp(entry->d_name, "hwmon", 5) != 0)
            continue;
        if (entry->d_name[5] < '0' || entry->d_name[5] > '9')
            continue;

        {
            size_t needed = strlen(work) + 1 + strlen(entry->d_name) + 1;
            resolved = malloc(needed);
            if (resolved)
                snprintf(resolved, needed, "%s/%s", work, entry->d_name);
        }
        break;
    }

    closedir(dir);

    if (!resolved)
        return glob_result ? glob_result : strdup(work);

    free(glob_result);
    DBG("hwmon: resolved %s -> %s\n", path, resolved);
    return resolved;
}
