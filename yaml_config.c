#include <stdio.h>
#include <stdlib.h>
#include <yaml.h>

#include "fan.h"
#include "zone.h"
#include "sensor.h"
#include "curve.h"
#include "config.h"
#include "common.h"

#define INDENT "  "
#define STRVAL(x) ((x) ? (char*)(x) : "")

void indent(int level)
{
    int i;
    for (i = 0; i < level; i++) {
        printf("%s", INDENT);
    }
}

void print_event(yaml_event_t *event)
{
    static int level = 0;

    switch (event->type) {
    case YAML_NO_EVENT:
        indent(level);
        printf("no-event (%d)\n", event->type);
        break;
    case YAML_STREAM_START_EVENT:
        indent(level++);
        printf("stream-start-event (%d)\n", event->type);
        break;
    case YAML_STREAM_END_EVENT:
        indent(--level);
        printf("stream-end-event (%d)\n", event->type);
        break;
    case YAML_DOCUMENT_START_EVENT:
        indent(level++);
        printf("document-start-event (%d)\n", event->type);
        break;
    case YAML_DOCUMENT_END_EVENT:
        indent(--level);
        printf("document-end-event (%d)\n", event->type);
        break;
    case YAML_ALIAS_EVENT:
        indent(level);
        printf("alias-event (%d)\n", event->type);
        break;
    case YAML_SCALAR_EVENT:
        indent(level);
        printf("scalar-event (%d) = {value=\"%s\", length=%d}\n",
               event->type,
               STRVAL(event->data.scalar.value),
               (int)event->data.scalar.length);
        break;
    case YAML_SEQUENCE_START_EVENT:
        indent(level++);
        printf("sequence-start-event (%d)\n", event->type);
        break;
    case YAML_SEQUENCE_END_EVENT:
        indent(--level);
        printf("sequence-end-event (%d)\n", event->type);
        break;
    case YAML_MAPPING_START_EVENT:
        indent(level++);
        printf("mapping-start-event (%d)\n", event->type);
        break;
    case YAML_MAPPING_END_EVENT:
        indent(--level);
        printf("mapping-end-event (%d)\n", event->type);
        break;
    }
    if (level < 0) {
        fprintf(stderr, "indentation underflow!\n");
        level = 0;
    }
}


struct fand_config *fand_config_load(const char *cfg_path)
{
    struct fand_config *cfg = NULL;

    yaml_parser_t parser;
    yaml_event_t event;

    int done = 0;

    /* Create the Parser object. */
    yaml_parser_initialize(&parser);

    FILE *input = fopen(cfg_path, "rb");
    yaml_parser_set_input_file(&parser, input);

    /* Read the event sequence. */
    while (!done) {

        /* Get the next event. */
        if (!yaml_parser_parse(&parser, &event)) {
            printf("Config file malformed at line %ld, col %ld: %s\n",
                parser.problem_mark.line, parser.problem_mark.column, parser.problem);
            return NULL;
        }

        print_event(&event);

        /* Are we finished? */
        done = (event.type == YAML_STREAM_END_EVENT);

        /* The application is responsible for destroying the event object. */
        yaml_event_delete(&event);
    }

    return cfg;
}

void fand_config_enable(struct fand_config *cfg)
{
    return;
}

void fand_config_disable(struct fand_config *cfg)
{
    return;
}

void fand_config_destroy(struct fand_config *cfg)
{
    return;
}

