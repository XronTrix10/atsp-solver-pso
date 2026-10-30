#include "particle.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "tour_utils.h"

int particle_init(Particle *p, const Problem *pr, uint64_t seed, uint64_t stream)
{
    const int n = pr->n;

    p->n = n;
    p->position = malloc((size_t)n * sizeof(int));
    p->best_position = malloc((size_t)n * sizeof(int));
    p->velocity = malloc((size_t)n * sizeof(double));
    if (!p->position || !p->best_position || !p->velocity)
    {
        particle_free(p);
        return -1;
    }

    rng_seed(&p->rng, seed, stream);

    /* Random permutation (Fisher-Yates) */
    for (int i = 0; i < n; i++)
        p->position[i] = i;
    for (int i = n - 1; i > 0; i--)
        tour_swap(p->position, i, rng_int(&p->rng, i + 1));
    memcpy(p->best_position, p->position, (size_t)n * sizeof(int));

    /* Random velocity in [0, 1) */
    double sum = 0;
    for (int i = 0; i < n; i++)
    {
        p->velocity[i] = rng_double(&p->rng);
        sum += p->velocity[i];
    }
    p->avg_velocity = sum / n;

    p->best_fit = LONG_MAX;
    p->current_fit = problem_tour_length(pr, p->position);
    return 0;
}

void particle_free(Particle *p)
{
    free(p->position);
    free(p->best_position);
    free(p->velocity);
    p->position = NULL;
    p->best_position = NULL;
    p->velocity = NULL;
}

void particle_update_local_best(Particle *p)
{
    if (p->current_fit < p->best_fit)
    {
        p->best_fit = p->current_fit;
        memcpy(p->best_position, p->position, (size_t)p->n * sizeof(int));
    }
}

void particle_update_velocity(Particle *p, const int *global_best_position,
                              double w, double c1, double c2)
{
    double sum = 0;
    for (int j = 0; j < p->n; j++)
    {
        double r1 = rng_double(&p->rng);
        double r2 = rng_double(&p->rng);

        p->velocity[j] =
            w * p->velocity[j] +
            c1 * r1 * (p->best_position[j] - p->position[j]) +
            c2 * r2 * (global_best_position[j] - p->position[j]);

        sum += p->velocity[j];
    }
    p->avg_velocity = sum / p->n;
}
