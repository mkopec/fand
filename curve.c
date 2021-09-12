#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "curve.h"

int curve_get_value (struct curve *c, float in)
{
    int i;
    float slope;
    
    // min speed
    if (in <= c->inputs[0]) 
        return c->outputs[0];

    // on the curve
    for (i = 0; i < c->length - 1; ++i)
    {
        // on a curve point
        if (in == c->inputs[i])
            return c->outputs[i];

        // inbetween curve points
        if (in > c->inputs[i] && in < c->inputs[i+1])
        {
            slope = (float)(c->outputs[i+1] - c->outputs[i]) / (c->inputs[i+1] - c->inputs[i]);
            return slope * (in - c->inputs[i]) + c->outputs[i];
        }
    }

    // max speed
    return c->outputs[c->length - 1];
}

struct curve *curve_create (int *inputs, int *outputs, int length)
{
    struct curve *c = malloc(sizeof(struct curve));

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