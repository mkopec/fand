#include "common.h"

struct curve {
    int     *inputs;
    int     *outputs;
    int     length;
};

int curve_get_value (struct curve *c, float in);
struct curve *curve_create (int *inputs, int *outputs, int length);
void curve_destroy (struct curve *c);