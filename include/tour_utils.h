#ifndef PSO_TOUR_UTILS_H
#define PSO_TOUR_UTILS_H

#include <stdbool.h>

#include "rng.h"

/* Small helpers operating on a tour (permutation of 0..n-1). */

static inline void tour_swap(int *tour, int i, int j)
{
    int tmp = tour[i];
    tour[i] = tour[j];
    tour[j] = tmp;
}

/* Reverse the circular segment that starts at index i and ends at index j
 * (walking forward, wrapping around the end of the array if j < i). */
void tour_reverse(int *tour, int n, int i, int j);

/* Copy `tour` into `out` and reverse a random segment of `out`. */
void tour_inverse_mutation(const int *tour, int *out, int n, Rng *rng);

/* True if tour is a valid permutation of 0..n-1. */
bool tour_is_valid(const int *tour, int n);

#endif
