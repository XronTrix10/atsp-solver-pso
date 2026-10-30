#include "local_search.h"

#include <limits.h>
#include <math.h>

#include "tour_utils.h"

#define COST(a, b) problem_cost(pr, (a), (b))

/* ===================================
 * Delta evaluation
 * =================================== */

long swap_delta_cost(const Problem *pr, const int *pos, int i, int j)
{
    const int n = pr->n;

    if (i < 0 || i >= n || j < 0 || j >= n || i == j)
        return 0;

    /* Ensure i < j to simplify adjacency logic */
    if (i > j)
    {
        int tmp = i;
        i = j;
        j = tmp;
    }

    const int prev_i = (i - 1 + n) % n;
    const int next_i = (i + 1) % n;
    const int prev_j = (j - 1 + n) % n;
    const int next_j = (j + 1) % n;

    long old_cost, new_cost;

    if (next_i == j)
    {
        /* Case 1: adjacent nodes (i is right before j) */
        old_cost = COST(pos[prev_i], pos[i]) +
                   COST(pos[i], pos[j]) +
                   COST(pos[j], pos[next_j]);

        new_cost = COST(pos[prev_i], pos[j]) +
                   COST(pos[j], pos[i]) +
                   COST(pos[i], pos[next_j]);
    }
    else if (next_j == i)
    {
        /* Case 2: adjacent wrap-around (j at the end, i at the start) */
        old_cost = COST(pos[prev_j], pos[j]) +
                   COST(pos[j], pos[i]) +
                   COST(pos[i], pos[next_i]);

        new_cost = COST(pos[prev_j], pos[i]) +
                   COST(pos[i], pos[j]) +
                   COST(pos[j], pos[next_i]);
    }
    else
    {
        /* Case 3: non-adjacent standard swap */
        old_cost = COST(pos[prev_i], pos[i]) + COST(pos[i], pos[next_i]) +
                   COST(pos[prev_j], pos[j]) + COST(pos[j], pos[next_j]);

        new_cost = COST(pos[prev_i], pos[j]) + COST(pos[j], pos[next_i]) +
                   COST(pos[prev_j], pos[i]) + COST(pos[i], pos[next_j]);
    }

    return new_cost - old_cost;
}

long reversal_delta_cost(const Problem *pr, const int *path, int i, int j)
{
    const int n = pr->n;
    const int prev_i = (i - 1 + n) % n;
    const int next_j = (j + 1) % n;

    /* Endpoint edges */
    long old_cost = COST(path[prev_i], path[i]) + COST(path[j], path[next_j]);
    long new_cost = COST(path[prev_i], path[j]) + COST(path[i], path[next_j]);

    /* ATSP: reversing a path flips the direction of every internal edge,
     * so the internal edges must be re-costed in the opposite direction. */
    const int count = (i <= j) ? (j - i) : (n - i + j);
    int curr = i;

    for (int k = 0; k < count; k++)
    {
        int next = (curr + 1) % n;
        old_cost += COST(path[curr], path[next]);
        new_cost += COST(path[next], path[curr]);
        curr = next;
    }

    return new_cost - old_cost;
}

/* ===================================
 * Greedy edge swap
 * =================================== */

void greedy_edge_swap(const Problem *pr, Particle *p)
{
    const int n = pr->n;

    for (int i = 0; i < n - 2; i++)
    {
        for (int j = i + 1; j < n - 1; j++)
        {
            long delta = swap_delta_cost(pr, p->position, i, j);
            if (delta < 0)
            {
                tour_swap(p->position, i, j);
                p->current_fit += delta; /* O(1) fitness update */
            }
        }
    }
}

/* ===================================
 * Simulated annealing
 * =================================== */

/* Pick two distinct indices. A segment i..j with j == i-1 (mod n) would
 * reverse the whole cycle, a case reversal_delta_cost() does not model
 * (the closing edge is counted twice), so it is excluded. */
static void pick_reversal(Rng *rng, int n, int *i, int *j)
{
    int a, b;
    do
    {
        a = rng_int(rng, n);
        b = rng_int(rng, n);
    } while (a == b || b == (a + n - 1) % n);

    *i = a;
    *j = b;
}

void simulated_annealing(const Problem *pr, Particle *p,
                         double temp, double cooling_rate)
{
    const int n = pr->n;

    while (temp > 1)
    {
        int i, j;
        pick_reversal(&p->rng, n, &i, &j);

        /* Cost difference of the candidate move */
        long delta = reversal_delta_cost(pr, p->position, i, j);

        /* Accept if better, or with Metropolis probability */
        if (delta < 0 || rng_double(&p->rng) < exp(-(double)delta / temp))
        {
            /* Lazy mutation: only touch the tour when the move is accepted */
            tour_reverse(p->position, n, i, j);
            p->current_fit += delta;
        }

        temp *= cooling_rate;
    }
}

/* ===================================
 * RGIBNNM (nearest-neighbour insertion)
 * =================================== */

void rgibnnm(const Problem *pr, Particle *p)
{
    const int n_dim = pr->n;
    int *pos = p->position;
    int x = 0, improved = 0;

    while (!improved && x < n_dim)
    {
        int gene = x;
        int nearest = -1;
        int min_dist = INT_MAX;

        /* Find the nearest neighbour of the selected gene */
        for (int i = 0; i < n_dim; i++)
        {
            if (i != gene)
            {
                int dist = COST(pos[gene], pos[i]);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    nearest = i;
                }
            }
        }

        if (nearest != -1)
        {
            int prev_g = (gene - 1 + n_dim) % n_dim;

            /* If the nearest neighbour is already right before the gene,
             * inserting it there changes nothing. */
            if (nearest != prev_g)
            {
                int next_g = (gene + 1) % n_dim;
                int next_n = (nearest + 1) % n_dim;
                long delta;

                if (next_g == nearest)
                {
                    /* gene and nearest are adjacent */
                    delta = COST(pos[prev_g], pos[nearest]) +
                            COST(pos[nearest], pos[gene]) +
                            COST(pos[gene], pos[next_n]) -
                            (COST(pos[prev_g], pos[gene]) +
                             COST(pos[gene], pos[nearest]) +
                             COST(pos[nearest], pos[next_n]));
                }
                else
                {
                    /* removed edges vs added edges */
                    delta = COST(pos[prev_g], pos[next_g]) +
                            COST(pos[nearest], pos[gene]) +
                            COST(pos[gene], pos[next_n]) -
                            (COST(pos[prev_g], pos[gene]) +
                             COST(pos[gene], pos[next_g]) +
                             COST(pos[nearest], pos[next_n]));
                }

                if (delta < 0)
                {
                    int temp = pos[gene];

                    /* Apply array shift in place */
                    if (gene < nearest)
                    {
                        for (int i = gene; i < nearest; i++)
                            pos[i] = pos[i + 1];
                        pos[nearest] = temp;
                    }
                    else
                    {
                        for (int i = gene; i > nearest + 1; i--)
                            pos[i] = pos[i - 1];
                        pos[nearest + 1] = temp;
                    }

                    p->current_fit += delta;
                    improved = 1;
                }
            }
        }

        if (!improved)
            x++;
    }
}
