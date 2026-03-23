#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "curve.h"

static int clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

int curve_get_value (struct curve *c, float in)
{
    int i;
    float slope;

    // min speed
    if (in <= c->inputs[0])
        return clamp(c->outputs[0], 0, 255);

    // on the curve
    for (i = 0; i < c->length - 1; ++i)
    {
        // on a curve point
        if (in == c->inputs[i])
            return clamp(c->outputs[i], 0, 255);

        // inbetween curve points
        if (in > c->inputs[i] && in < c->inputs[i+1])
        {
            slope = (float)(c->outputs[i+1] - c->outputs[i]) / (c->inputs[i+1] - c->inputs[i]);
            return clamp((int)(slope * (in - c->inputs[i]) + c->outputs[i]), 0, 255);
        }
    }

    // max speed
    return clamp(c->outputs[c->length - 1], 0, 255);
}

struct curve *curve_create (int *inputs, int *outputs, int length)
{
    int i;
    struct curve *c;

    if (length < 1)
        return NULL;
    for (i = 0; i < length - 1; ++i) {
        if (inputs[i] >= inputs[i+1]) {
            DBG("curve: temperatures must be strictly increasing\n");
            return NULL;
        }
    }

    c = malloc(sizeof(struct curve));

    if (!c)
        return NULL;

    c->length = length;

    c->inputs = malloc(length * sizeof(int));
    c->outputs = malloc(length * sizeof(int));

    memcpy(c->inputs, inputs, length * sizeof(int));
    memcpy(c->outputs, outputs, length * sizeof(int));

    return c;
}

void curve_destroy (struct curve *c)
{
    free (c->inputs);
    free (c->outputs);
    free (c);
}
