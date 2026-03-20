/*
 * Resolve a stable hwmon device path to the actual hwmonX path.
 *
 * If `path` ends with "/hwmon" (i.e. it is the hwmon parent directory inside
 * a device tree entry such as /sys/devices/platform/asus-ec-sensors/hwmon),
 * the directory is scanned for the first hwmonN child and its full path is
 * returned.  For any other path the function returns a copy of the original
 * string unchanged, preserving backward compatibility with paths that already
 * contain a concrete hwmon number.
 *
 * The caller is responsible for freeing the returned string.
 * Returns NULL on allocation failure.
 */
char *hwmon_resolve_path(const char *path);
