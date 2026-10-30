#ifndef PSO_CONFIG_H
#define PSO_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* ---- Defaults for optional CLI parameters ---- */
#define DEFAULT_SWARM 60
#define DEFAULT_W     0.5   /* inertia weight       */
#define DEFAULT_C1    2.0   /* acceleration factor  */
#define DEFAULT_C2    2.0   /* acceleration factor  */
#define DEFAULT_SEED  1ULL

/* ---- Fixed algorithm constants (not exposed on the CLI) ---- */
#define SA_INITIAL_TEMP  100.0
#define SA_COOLING_RATE  0.99
#define MIN_DIMENSION    4
#define OUTPUT_PATH_MAX  1024
#define OUTPUT_DIR       "solution"

/*
 * The original code had the velocity update commented out (positions are
 * driven purely by the local-search operators), so w/c1/c2 have no effect
 * unless the program is built with -DPSO_ENABLE_VELOCITY (make VELOCITY=1).
 */
#ifdef PSO_ENABLE_VELOCITY
#define PSO_VELOCITY_ENABLED 1
#else
#define PSO_VELOCITY_ENABLED 0
#endif

typedef struct
{
    /* mandatory */
    const char *input_path;   /* cost-matrix file               */
    int dim;                  /* number of cities / matrix size */

    /* derived by the driver: solution/<input name>_<date-time>.log */
    char output_path[OUTPUT_PATH_MAX];

    /* optional */
    int swarm;
    double w, c1, c2;
    uint64_t seed;

    /* stop conditions (at least one must be set) */
    bool has_optimal;
    long optimal;             /* stop once best fitness <= optimal */
    bool has_max_iter;
    long max_iter;            /* stop after this many iterations   */
} Config;

void config_set_defaults(Config *cfg);
void config_print(FILE *out, const Config *cfg);

#endif
