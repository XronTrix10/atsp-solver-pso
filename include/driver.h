#ifndef PSO_DRIVER_H
#define PSO_DRIVER_H

#include "config.h"

/* Load the problem, run the PSO, report and log the result.
 * Returns EXIT_SUCCESS or EXIT_FAILURE. */
int driver_run(const Config *cfg);

#endif
