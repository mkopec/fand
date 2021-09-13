#define FAND_VERSION "0.1.0"

#define MAX_PATH 255
#define MAX_ZONE_SIZE 20
#define MAX_ZONES 20

#define DEBUG 1
#define DBG(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt  __VA_OPT__(,) __VA_ARGS__); } while (0)
