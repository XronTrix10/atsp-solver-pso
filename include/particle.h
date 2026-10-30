#ifndef PSO_PARTICLE_H
#define PSO_PARTICLE_H

#include <stdint.h>

#include "problem.h"
#include "rng.h"

typedef struct
{
    int n;
    int *position;      /* current tour                      */
    double *velocity;
    double avg_velocity;
    int *best_position; /* best tour this particle has seen  */
    long best_fit;
    long current_fit;
    Rng rng;            /* private random stream             */
} Particle;

/* Random initial tour/velocity. `stream` selects this particle's RNG stream. */
int particle_init(Particle *p, const Problem *pr, uint64_t seed, uint64_t stream);
void particle_free(Particle *p);

/* Update the particle's personal best from its current position. */
void particle_update_local_best(Particle *p);

/* Classic PSO velocity update (only used when built with VELOCITY=1). */
void particle_update_velocity(Particle *p, const int *global_best_position,
                              double w, double c1, double c2);

#endif
