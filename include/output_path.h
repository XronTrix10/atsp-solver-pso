#ifndef PSO_OUTPUT_PATH_H
#define PSO_OUTPUT_PATH_H

#include <stddef.h>

/*
 * Build the result-file path  solution/<input name>_<YYYY-MM-DD_HH-MM-SS>.log
 * where <input name> is the input file's base name without its extension.
 * Creates the "solution" directory (in the current directory) if it does not
 * exist. Returns 0 on success, -1 on error (message printed to stderr).
 */
int output_path_build(char *buf, size_t len, const char *input_path);

#endif
