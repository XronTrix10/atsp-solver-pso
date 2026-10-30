#include "rng.h"

static uint64_t splitmix64(uint64_t *state)
{
    uint64_t z = (*state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

void rng_seed(Rng *r, uint64_t seed, uint64_t stream)
{
    uint64_t sm = seed;
    uint64_t base = splitmix64(&sm); /* decorrelate the raw user seed */

    /* decorrelate streams so neighbouring particles don't share sequences */
    sm = base ^ (stream * 0xD1B54A32D192ED03ULL + 0x2545F4914F6CDD1DULL);

    for (int i = 0; i < 4; i++)
        r->s[i] = splitmix64(&sm);

    if ((r->s[0] | r->s[1] | r->s[2] | r->s[3]) == 0)
        r->s[0] = 1; /* all-zero state is invalid for xoshiro */
}
