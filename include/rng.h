#ifndef PSO_RNG_H
#define PSO_RNG_H

#include <stdint.h>

/*
 * Small, fast, per-owner pseudo random generator (xoshiro256**).
 * Every particle owns one, so no global/shared RNG state is touched from
 * OpenMP threads and a given seed gives reproducible per-particle streams.
 */
typedef struct
{
    uint64_t s[4];
} Rng;

/* Seed generator `stream` of the family identified by `seed`. */
void rng_seed(Rng *r, uint64_t seed, uint64_t stream);

static inline uint64_t rng_rotl(uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static inline uint64_t rng_next(Rng *r)
{
    uint64_t *s = r->s;
    const uint64_t result = rng_rotl(s[1] * 5, 7) * 9;
    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rng_rotl(s[3], 45);
    return result;
}

/* Uniform integer in [0, n). */
static inline int rng_int(Rng *r, int n)
{
    return (int)(((rng_next(r) >> 32) * (uint64_t)n) >> 32);
}

/* Uniform double in [0, 1). */
static inline double rng_double(Rng *r)
{
    return (double)(rng_next(r) >> 11) * (1.0 / 9007199254740992.0);
}

#endif
