#include "tour_utils.h"

#include <stdlib.h>
#include <string.h>

void tour_reverse(int *tour, int n, int i, int j)
{
    /* number of elements in the (possibly wrapping) segment */
    int count = (i <= j) ? (j - i + 1) : (n - i + j + 1);
    int swaps = count / 2;

    for (int k = 0; k < swaps; k++)
    {
        int a = (i + k) % n;
        int b = (j - k + n) % n; /* +n prevents negative modulo in C */
        tour_swap(tour, a, b);
    }
}

void tour_inverse_mutation(const int *tour, int *out, int n, Rng *rng)
{
    memcpy(out, tour, (size_t)n * sizeof(int));

    int lower = rng_int(rng, n);
    int upper = rng_int(rng, n);
    while (lower == upper)
        upper = rng_int(rng, n);

    tour_reverse(out, n, lower, upper);
}

bool tour_is_valid(const int *tour, int n)
{
    bool ok = true;
    bool *seen = calloc((size_t)n, sizeof *seen);
    if (seen == NULL)
        return false;

    for (int i = 0; i < n && ok; i++)
    {
        if (tour[i] < 0 || tour[i] >= n || seen[tour[i]])
            ok = false;
        else
            seen[tour[i]] = true;
    }

    free(seen);
    return ok;
}
