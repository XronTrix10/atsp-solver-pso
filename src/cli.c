#include "cli.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

enum
{
    OPT_C1 = 1001,
    OPT_C2
};

static const char *short_opts = "i:d:s:w:r:O:m:h";

static const struct option long_opts[] = {
    {"input", required_argument, NULL, 'i'},
    {"dim", required_argument, NULL, 'd'},
    {"swarm", required_argument, NULL, 's'},
    {"inertia", required_argument, NULL, 'w'},
    {"c1", required_argument, NULL, OPT_C1},
    {"c2", required_argument, NULL, OPT_C2},
    {"seed", required_argument, NULL, 'r'},
    {"optimal", required_argument, NULL, 'O'},
    {"max-iter", required_argument, NULL, 'm'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

static const char *prog_name(const char *argv0)
{
    const char *slash = strrchr(argv0, '/');
    return slash ? slash + 1 : argv0;
}

void cli_print_usage(FILE *out, const char *prog)
{
    fprintf(out,
            "Usage: %s -i FILE -d N (--optimal VALUE | --max-iter N) [options]\n"
            "\n"
            "Hybrid PSO solver (greedy edge swap + simulated annealing + RGIBNNM)\n"
            "for tour problems given as a dim x dim cost matrix.\n"
            "\n"
            "Required:\n"
            "  -i, --input FILE      cost-matrix file (dim x dim whitespace-separated integers)\n"
            "  -d, --dim N           matrix size / number of cities (integer >= %d)\n"
            "\n"
            "Stop condition (at least one required; the run stops at whichever comes first):\n"
            "  -O, --optimal VALUE   run until best fitness <= VALUE (runs forever if never reached)\n"
            "  -m, --max-iter N      run for at most N iterations (N >= 1)\n"
            "\n"
            "Optional:\n"
            "  -s, --swarm N         number of particles            (default: %d)\n"
            "  -w, --inertia W       inertia weight w               (default: %g)\n"
            "      --c1 X            cognitive acceleration factor  (default: %g)\n"
            "      --c2 X            social acceleration factor     (default: %g)\n"
            "  -r, --seed N          random seed, unsigned integer  (default: %llu)\n"
            "  -h, --help            show this help and exit\n"
            "\n"
            "Output: the log and final result go to %s/<input name>_<date-time>.log\n"
            "in the current directory (the directory is created if missing).\n"
            "\n"
            "Note: w, c1 and c2 only take effect in a build with the velocity update\n"
            "enabled (make VELOCITY=1); the default build ignores them.\n"
            "Press Ctrl-C to stop a running search; the best tour so far is reported.\n"
            "\n"
            "Examples:\n"
            "  %s -i ftv44.txt -d 45 --optimal 1613\n"
            "  %s -i ftv44.txt -d 45 -m 5000 -s 100 -r 42\n"
            "  %s -i ftv44.txt -d 45 -O 1613 -m 20000   # whichever first\n",
            prog, MIN_DIMENSION, DEFAULT_SWARM, DEFAULT_W, DEFAULT_C1, DEFAULT_C2,
            (unsigned long long)DEFAULT_SEED, OUTPUT_DIR, prog, prog, prog);
}

/* Print "prog: error: ..." followed by the full usage text, to stderr. */
static CliStatus cli_fail(const char *prog, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "%s: error: ", prog);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n\n");
    va_end(ap);

    cli_print_usage(stderr, prog);
    return CLI_ERROR;
}

/* ---- strict number parsers: whole string must be consumed ---- */

static int parse_long(const char *s, long *out)
{
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (s[0] == '\0' || *end != '\0' || errno == ERANGE)
        return -1;
    *out = v;
    return 0;
}

static int parse_u64(const char *s, uint64_t *out)
{
    char *end;
    if (s[0] == '\0' || strchr(s, '-') != NULL) /* strtoull silently accepts negatives */
        return -1;
    errno = 0;
    unsigned long long v = strtoull(s, &end, 10);
    if (*end != '\0' || errno == ERANGE)
        return -1;
    *out = (uint64_t)v;
    return 0;
}

static int parse_double(const char *s, double *out)
{
    char *end;
    errno = 0;
    double v = strtod(s, &end);
    if (s[0] == '\0' || *end != '\0' || errno == ERANGE || !isfinite(v))
        return -1;
    *out = v;
    return 0;
}

CliStatus cli_parse(int argc, char **argv, Config *cfg)
{
    const char *prog = prog_name(argv[0]);
    config_set_defaults(cfg);

    bool have_dim = false;
    long lv;
    int opt;

    optind = 1;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
    {
        switch (opt)
        {
        case 'i':
            cfg->input_path = optarg;
            break;
        case 'd':
            if (parse_long(optarg, &lv) != 0 || lv < MIN_DIMENSION || lv > INT_MAX)
                return cli_fail(prog, "invalid dimension '%s' (expected integer >= %d)",
                                optarg, MIN_DIMENSION);
            cfg->dim = (int)lv;
            have_dim = true;
            break;
        case 's':
            if (parse_long(optarg, &lv) != 0 || lv < 1 || lv > INT_MAX)
                return cli_fail(prog, "invalid swarm size '%s' (expected integer >= 1)", optarg);
            cfg->swarm = (int)lv;
            break;
        case 'w':
            if (parse_double(optarg, &cfg->w) != 0)
                return cli_fail(prog, "invalid inertia weight '%s' (expected a number)", optarg);
            break;
        case OPT_C1:
            if (parse_double(optarg, &cfg->c1) != 0)
                return cli_fail(prog, "invalid c1 '%s' (expected a number)", optarg);
            break;
        case OPT_C2:
            if (parse_double(optarg, &cfg->c2) != 0)
                return cli_fail(prog, "invalid c2 '%s' (expected a number)", optarg);
            break;
        case 'r':
            if (parse_u64(optarg, &cfg->seed) != 0)
                return cli_fail(prog, "invalid seed '%s' (expected unsigned integer)", optarg);
            break;
        case 'O':
            if (parse_long(optarg, &cfg->optimal) != 0)
                return cli_fail(prog, "invalid optimal value '%s' (expected integer)", optarg);
            cfg->has_optimal = true;
            break;
        case 'm':
            if (parse_long(optarg, &cfg->max_iter) != 0 || cfg->max_iter < 1)
                return cli_fail(prog, "invalid max iterations '%s' (expected integer >= 1)", optarg);
            cfg->has_max_iter = true;
            break;
        case 'h':
            cli_print_usage(stdout, prog);
            return CLI_HELP;
        default: /* getopt already printed "unrecognized option" / "missing argument" */
            fprintf(stderr, "\n");
            cli_print_usage(stderr, prog);
            return CLI_ERROR;
        }
    }

    if (optind < argc)
        return cli_fail(prog, "unexpected argument '%s'", argv[optind]);

    /* mandatory arguments */
    if (cfg->input_path == NULL || !have_dim)
    {
        char missing[128] = "";
        if (cfg->input_path == NULL)
            strcat(missing, " --input");
        if (!have_dim)
            strcat(missing, " --dim");
        return cli_fail(prog, "missing required argument(s):%s", missing);
    }

    /* an unbounded run needs a way to end */
    if (!cfg->has_optimal && !cfg->has_max_iter)
        return cli_fail(prog, "no stop condition: give --optimal VALUE and/or --max-iter N");

    return CLI_OK;
}
