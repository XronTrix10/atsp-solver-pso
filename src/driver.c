#include "driver.h"

#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "output_path.h"
#include "problem.h"
#include "pso.h"
#include "tour_utils.h"

static void print_banner(const Config *cfg)
{
    printf("\n===================== PSO ALGORITHM ======================\n\n");
    config_print(stdout, cfg);
    printf("\n");
    fflush(stdout);
}

static void print_result(const PsoResult *res)
{
    printf("\nStopped: %s after %ld iterations (%.3f s)\n",
           pso_stop_reason_str(res->reason), res->iterations, res->elapsed_sec);
    printf("\nBest solution found with fitness: %ld\n\n\t", res->best_fit);
    for (int i = 0; i < res->n; i++)
        printf("%d ", res->best_tour[i]);
    printf("\n");
}

/* Cross-check the incrementally tracked fitness against a full recomputation. */
static int verify_result(const Problem *pr, const PsoResult *res, Logger *log)
{
    if (!tour_is_valid(res->best_tour, res->n))
    {
        fprintf(stderr, "error: internal check failed: best tour is not a valid permutation\n");
        logger_write_note(log, "ERROR: best tour is not a valid permutation");
        return -1;
    }

    long actual = problem_tour_length(pr, res->best_tour);
    if (actual != res->best_fit)
    {
        fprintf(stderr,
                "error: internal check failed: tracked fitness %ld != recomputed tour length %ld\n",
                res->best_fit, actual);
        logger_write_note(log, "ERROR: tracked fitness does not match recomputed tour length");
        return -1;
    }
    return 0;
}

int driver_run(const Config *user_cfg)
{
    Config cfg_copy = *user_cfg; /* output_path is derived below */
    const Config *cfg = &cfg_copy;
    int status = EXIT_FAILURE;
    Problem pr = {0};
    Logger log = {0};
    PsoResult res = {0};

    if (problem_load(&pr, cfg->input_path, cfg->dim) != 0)
        goto done;

    /* solution/<input name>_<date-time>.log (creates the directory) */
    if (output_path_build(cfg_copy.output_path, sizeof cfg_copy.output_path,
                          cfg->input_path) != 0)
        goto done;

    /* Open the output file up front so a bad path fails before a long run */
    if (logger_open(&log, cfg->output_path) != 0)
        goto done;
    logger_write_header(&log, cfg);

    print_banner(cfg);

    if (pso_run(&pr, cfg, &log, &res) != 0)
    {
        fprintf(stderr, "error: PSO run failed (out of memory)\n");
        goto done;
    }

    print_result(&res);
    logger_write_result(&log, res.best_fit, res.best_tour, res.n, res.iterations,
                        res.elapsed_sec, pso_stop_reason_str(res.reason));

    if (verify_result(&pr, &res, &log) == 0)
        status = EXIT_SUCCESS;

done:
    pso_result_free(&res);
    logger_close(&log);
    problem_free(&pr);
    return status;
}
