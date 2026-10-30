#ifndef PSO_PSO_H
#define PSO_PSO_H

#include "config.h"
#include "logger.h"
#include "problem.h"

typedef enum
{
    STOP_OPTIMAL,     /* best fitness reached the requested optimal value */
    STOP_MAX_ITER,    /* iteration limit reached                          */
    STOP_INTERRUPTED  /* SIGINT / SIGTERM received                        */
} StopReason;

typedef struct
{
    int n;
    long best_fit;
    int *best_tour;      /* n entries, owned by the result */
    long iterations;     /* completed iterations           */
    double elapsed_sec;
    StopReason reason;
} PsoResult;

/* Run the swarm. Returns 0 on success, -1 on allocation failure. */
int pso_run(const Problem *pr, const Config *cfg, Logger *log, PsoResult *out);
void pso_result_free(PsoResult *res);

/* Ask a running pso_run() to stop after the current iteration.
 * Async-signal-safe (just sets a flag). */
void pso_request_stop(void);

const char *pso_stop_reason_str(StopReason reason);

#endif
