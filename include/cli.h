#ifndef PSO_CLI_H
#define PSO_CLI_H

#include <stdio.h>

#include "config.h"

typedef enum
{
    CLI_OK,    /* arguments valid, cfg filled in                      */
    CLI_HELP,  /* --help requested; usage already printed to stdout   */
    CLI_ERROR  /* invalid arguments; message + usage printed to stderr */
} CliStatus;

CliStatus cli_parse(int argc, char **argv, Config *cfg);
void cli_print_usage(FILE *out, const char *prog);

#endif
