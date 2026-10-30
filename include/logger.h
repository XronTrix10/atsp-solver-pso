#ifndef PSO_LOGGER_H
#define PSO_LOGGER_H

#include <stdio.h>

#include "config.h"

/* Appends timestamped progress/result lines to the user's output file. */
typedef struct
{
    FILE *fp;
} Logger;

int logger_open(Logger *log, const char *path); /* 0 on success */
void logger_close(Logger *log);

void logger_write_header(Logger *log, const Config *cfg);

/* NOT thread-safe: call from a single thread or inside a critical section. */
void logger_log_improvement(Logger *log, long iteration, long fitness);

void logger_write_result(Logger *log, long best_fit, const int *tour, int n,
                         long iterations, double elapsed_sec,
                         const char *stop_reason);

void logger_write_note(Logger *log, const char *msg);

#endif
