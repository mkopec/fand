#define FAND_VERSION "0.1.1"

#define MAX_PATH 255
#define MAX_ZONE_SIZE 20
#define MAX_ZONES 20

#define DEBUG 1
#define DBG(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt  __VA_OPT__(,) __VA_ARGS__); } while (0)
#define ERROR(fmt, ...) \
            do { fprintf(stderr, fmt  __VA_OPT__(,) __VA_ARGS__); } while (0)
#define INFO(fmt, ...) \
            do { printf(fmt  __VA_OPT__(,) __VA_ARGS__); } while (0)
