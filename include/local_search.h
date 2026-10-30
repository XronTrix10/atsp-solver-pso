#ifndef PSO_LOCAL_SEARCH_H
#define PSO_LOCAL_SEARCH_H

#include "particle.h"
#include "problem.h"

/* O(1) change in tour length if positions i and j were swapped. */
long swap_delta_cost(const Problem *pr, const int *tour, int i, int j);

/* Change in tour length if the circular segment i..j were reversed
 * (asymmetric costs: internal edges flip direction, so O(segment)). */
long reversal_delta_cost(const Problem *pr, const int *tour, int i, int j);

/* Hill-climb with pairwise swaps; keeps p->current_fit up to date. */
void greedy_edge_swap(const Problem *pr, Particle *p);

/* Simulated annealing using random segment reversals. */
void simulated_annealing(const Problem *pr, Particle *p,
                         double temp, double cooling_rate);

/* Nearest-neighbour insertion move (RGIBNNM). */
void rgibnnm(const Problem *pr, Particle *p);

#endif
