#include "logger.h"

#include <errno.h>
#include <string.h>
#include <time.h>

static void timestamp(char *buf, size_t len)
{
    time_t t = time(NULL);
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tm_info);
}

int logger_open(Logger *log, const char *path)
{
    log->fp = fopen(path, "a");
    if (log->fp == NULL)
    {
        fprintf(stderr, "error: cannot open output file '%s': %s\n",
                path, strerror(errno));
        return -1;
    }
    return 0;
}

void logger_close(Logger *log)
{
    if (log->fp != NULL)
    {
        fclose(log->fp);
        log->fp = NULL;
    }
}

void logger_write_header(Logger *log, const Config *cfg)
{
    if (log->fp == NULL)
        return;

    char ts[64];
    timestamp(ts, sizeof ts);

    fprintf(log->fp, "\n========================================\n");
    fprintf(log->fp, "Run started at: %s\n", ts);
    fprintf(log->fp, "========================================\n");
    config_print(log->fp, cfg);
    fprintf(log->fp, "\n");
    fflush(log->fp);
}

void logger_log_improvement(Logger *log, long iteration, long fitness)
{
    if (log == NULL || log->fp == NULL)
        return;

    char ts[64];
    timestamp(ts, sizeof ts);

    fprintf(log->fp, "[%s] Iteration: %8ld \t Fitness: %5ld\n", ts, iteration, fitness);
    fflush(log->fp); /* keep the file current even if the run is killed */
}

void logger_write_result(Logger *log, long best_fit, const int *tour, int n,
                         long iterations, double elapsed_sec,
                         const char *stop_reason)
{
    if (log->fp == NULL)
        return;

    char ts[64];
    timestamp(ts, sizeof ts);

    fprintf(log->fp, "\n---------------- RESULT ----------------\n");
    fprintf(log->fp, "Finished at  : %s\n", ts);
    fprintf(log->fp, "Stop reason  : %s\n", stop_reason);
    fprintf(log->fp, "Iterations   : %ld\n", iterations);
    fprintf(log->fp, "Elapsed (s)  : %.3f\n", elapsed_sec);
    fprintf(log->fp, "Best fitness : %ld\n", best_fit);
    fprintf(log->fp, "Best tour    :");
    for (int i = 0; i < n; i++)
        fprintf(log->fp, " %d", tour[i]);
    fprintf(log->fp, "\n");
    fflush(log->fp);
}

void logger_write_note(Logger *log, const char *msg)
{
    if (log->fp == NULL)
        return;
    fprintf(log->fp, "%s\n", msg);
    fflush(log->fp);
}
