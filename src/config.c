 #include "config.h"

void config_set_defaults(Config *cfg)
{
    cfg->input_path = NULL;
    cfg->output_path[0] = '\0';
    cfg->dim = 0;

    cfg->swarm = DEFAULT_SWARM;
    cfg->w = DEFAULT_W;
    cfg->c1 = DEFAULT_C1;
    cfg->c2 = DEFAULT_C2;
    cfg->seed = DEFAULT_SEED;

    cfg->has_optimal = false;
    cfg->optimal = 0;
    cfg->has_max_iter = false;
    cfg->max_iter = 0;
}

void config_print(FILE *out, const Config *cfg)
{
    fprintf(out, "Input file    : %s\n", cfg->input_path);
    fprintf(out, "Output file   : %s\n", cfg->output_path);
    fprintf(out, "Dimension     : %d\n", cfg->dim);
    fprintf(out, "Swarm size    : %d\n", cfg->swarm);
    fprintf(out, "w / c1 / c2   : %g / %g / %g%s\n", cfg->w, cfg->c1, cfg->c2,
            PSO_VELOCITY_ENABLED ? "" : "  (velocity update disabled: unused)");
    fprintf(out, "Seed          : %llu\n", (unsigned long long)cfg->seed);

    if (cfg->has_optimal)
        fprintf(out, "Optimal value : %ld\n", cfg->optimal);
    else
        fprintf(out, "Optimal value : (not set)\n");

    if (cfg->has_max_iter)
        fprintf(out, "Max iters     : %ld\n", cfg->max_iter);
    else
        fprintf(out, "Max iters     : (unlimited)\n");
}
