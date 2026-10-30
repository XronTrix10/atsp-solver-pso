#include "pso.h"

#include <limits.h>
#include <omp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "local_search.h"
#include "particle.h"

static volatile sig_atomic_t g_stop_requested = 0;

void pso_request_stop(void)
{
    g_stop_requested = 1;
}

const char *pso_stop_reason_str(StopReason reason)
{
    switch (reason)
    {
    case STOP_OPTIMAL:
        return "optimal value reached";
    case STOP_MAX_ITER:
        return "maximum iterations reached";
    case STOP_INTERRUPTED:
        return "interrupted by signal";
    }
    return "unknown";
}

void pso_result_free(PsoResult *res)
{
    free(res->best_tour);
    res->best_tour = NULL;
}

typedef struct
{
    long fit;
    int *position;
} GlobalBest;

/* Update personal best, then the global best (thread-safe). */
static void rank_particle(Particle *p, GlobalBest *g, long iteration, Logger *log)
{
    particle_update_local_best(p);

    long gfit;
#pragma omp atomic read
    gfit = g->fit;

    if (p->current_fit < gfit)
    {
        /* Prevent races when several threads find a new best simultaneously */
#pragma omp critical(pso_global_best)
        {
            /* Re-check: another thread may have beaten us to it */
            if (p->current_fit < g->fit)
            {
#pragma omp atomic write
                g->fit = p->current_fit;

                memcpy(g->position, p->position, (size_t)p->n * sizeof(int));
                logger_log_improvement(log, iteration, g->fit);
            }
        }
    }
}

/* Local-search pipeline that moves a particle to a new position. */
static void update_position(const Problem *pr, Particle *p)
{
    greedy_edge_swap(pr, p);
    if (p->best_fit > p->current_fit)
        return;

    simulated_annealing(pr, p, SA_INITIAL_TEMP, SA_COOLING_RATE);
    greedy_edge_swap(pr, p);
    if (p->best_fit > p->current_fit)
        return;

    rgibnnm(pr, p);
    greedy_edge_swap(pr, p);
}

int pso_run(const Problem *pr, const Config *cfg, Logger *log, PsoResult *out)
{
    const int n = pr->n;
    const int swarm = cfg->swarm;
    int rc = -1;

    memset(out, 0, sizeof *out);

    Particle *particles = calloc((size_t)swarm, sizeof *particles);
    GlobalBest g = {LONG_MAX, malloc((size_t)n * sizeof(int))};
    int *g_snapshot = malloc((size_t)n * sizeof(int)); /* race-free copy for velocity update */
    int initialised = 0;

    if (!particles || !g.position || !g_snapshot)
        goto cleanup;

    const double t_start = omp_get_wtime();

    /* Initialise swarm; each particle gets its own RNG stream */
    for (int i = 0; i < swarm; i++)
    {
        if (particle_init(&particles[i], pr, cfg->seed, (uint64_t)i) != 0)
            goto cleanup;
        initialised++;
        rank_particle(&particles[i], &g, 0, log);
    }

    long iter = 0;
    StopReason reason;

    for (;;)
    {
        if (cfg->has_optimal && g.fit <= cfg->optimal)
        {
            reason = STOP_OPTIMAL;
            break;
        }
        if (cfg->has_max_iter && iter >= cfg->max_iter)
        {
            reason = STOP_MAX_ITER;
            break;
        }
        if (g_stop_requested)
        {
            reason = STOP_INTERRUPTED;
            break;
        }

        iter++;

        if (PSO_VELOCITY_ENABLED)
            memcpy(g_snapshot, g.position, (size_t)n * sizeof(int));

#pragma omp parallel for
        for (int i = 0; i < swarm; i++)
        {
            Particle *p = &particles[i];

            if (PSO_VELOCITY_ENABLED)
                particle_update_velocity(p, g_snapshot, cfg->w, cfg->c1, cfg->c2);

            update_position(pr, p);
            rank_particle(p, &g, iter, log);
        }
    }

    out->n = n;
    out->best_fit = g.fit;
    out->best_tour = malloc((size_t)n * sizeof(int));
    if (out->best_tour == NULL)
        goto cleanup;
    memcpy(out->best_tour, g.position, (size_t)n * sizeof(int));
    out->iterations = iter;
    out->elapsed_sec = omp_get_wtime() - t_start;
    out->reason = reason;
    rc = 0;

cleanup:
    for (int i = 0; i < initialised; i++)
        particle_free(&particles[i]);
    free(particles);
    free(g.position);
    free(g_snapshot);
    return rc;
}
