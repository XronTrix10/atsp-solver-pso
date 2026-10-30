#ifndef PSO_PROBLEM_H
#define PSO_PROBLEM_H

#include <stddef.h>

/* Cost matrix of an (asymmetric) tour problem, stored row-major. */
typedef struct
{
    int n;
    int *cost; /* n * n entries */
} Problem;

/* Load an n x n whitespace-separated integer matrix. Returns 0 on success. */
int problem_load(Problem *pr, const char *path, int n);
void problem_free(Problem *pr);

static inline int problem_cost(const Problem *pr, int from, int to)
{
    return pr->cost[(size_t)from * (size_t)pr->n + (size_t)to];
}

/* Length of the closed tour visiting tour[0..n-1] and returning to tour[0]. */
long problem_tour_length(const Problem *pr, const int *tour);

#endif
