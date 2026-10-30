#include "problem.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int problem_load(Problem *pr, const char *path, int n)
{
    pr->n = 0;
    pr->cost = NULL;

    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open data file '%s': %s\n",
                path, strerror(errno));
        return -1;
    }

    const size_t total = (size_t)n * (size_t)n;
    int *cost = malloc(total * sizeof *cost);
    if (cost == NULL)
    {
        fprintf(stderr, "error: out of memory allocating %d x %d matrix\n", n, n);
        fclose(fp);
        return -1;
    }

    for (size_t k = 0; k < total; k++)
    {
        if (fscanf(fp, "%d", &cost[k]) != 1)
        {
            fprintf(stderr,
                    "error: failed to read data file '%s': expected %d x %d = %zu "
                    "integers but only found %zu (is --dim correct?)\n",
                    path, n, n, total, k);
            free(cost);
            fclose(fp);
            return -1;
        }
    }

    int extra;
    if (fscanf(fp, "%d", &extra) == 1)
        fprintf(stderr,
                "warning: data file '%s' holds more than %d x %d values; "
                "extra values ignored (is --dim correct?)\n",
                path, n, n);

    fclose(fp);
    pr->n = n;
    pr->cost = cost;
    return 0;
}

void problem_free(Problem *pr)
{
    free(pr->cost);
    pr->cost = NULL;
    pr->n = 0;
}

long problem_tour_length(const Problem *pr, const int *tour)
{
    long len = 0;
    for (int i = 0; i < pr->n - 1; i++)
        len += problem_cost(pr, tour[i], tour[i + 1]);
    len += problem_cost(pr, tour[pr->n - 1], tour[0]);
    return len;
}
