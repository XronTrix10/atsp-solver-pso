#include "output_path.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "config.h"

/* Base name of `path` without directory and last extension. */
static int input_stem(const char *path, char *stem, size_t len)
{
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;

    size_t n = strlen(base);
    const char *dot = strrchr(base, '.');
    if (dot != NULL && dot != base) /* keep dotfiles like ".data" whole */
        n = (size_t)(dot - base);

    if (n == 0 || n >= len)
        return -1;

    memcpy(stem, base, n);
    stem[n] = '\0';
    return 0;
}

static int ensure_directory(const char *dir)
{
    if (mkdir(dir, 0777) == 0)
        return 0;

    struct stat st;
    if (errno == EEXIST && stat(dir, &st) == 0 && S_ISDIR(st.st_mode))
        return 0;

    fprintf(stderr, "error: cannot create directory '%s': %s\n",
            dir, errno == EEXIST ? "exists and is not a directory" : strerror(errno));
    return -1;
}

int output_path_build(char *buf, size_t len, const char *input_path)
{
    char stem[256];
    if (input_stem(input_path, stem, sizeof stem) != 0)
    {
        fprintf(stderr, "error: cannot derive an output name from input path '%s'\n",
                input_path);
        return -1;
    }

    if (ensure_directory(OUTPUT_DIR) != 0)
        return -1;

    /* ':' avoided on purpose so the name is valid on every filesystem */
    char stamp[32];
    time_t t = time(NULL);
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    strftime(stamp, sizeof stamp, "%Y-%m-%d_%H-%M-%S", &tm_info);

    int w = snprintf(buf, len, "%s/%s_%s.log", OUTPUT_DIR, stem, stamp);
    if (w < 0 || (size_t)w >= len)
    {
        fprintf(stderr, "error: output path too long\n");
        return -1;
    }
    return 0;
}
