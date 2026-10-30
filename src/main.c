#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "driver.h"
#include "pso.h"

static void on_signal(int sig)
{
    (void)sig;
    pso_request_stop();
}

static void install_signal_handlers(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char **argv)
{
    Config cfg;

    switch (cli_parse(argc, argv, &cfg))
    {
    case CLI_OK:
        break;
    case CLI_HELP:
        return EXIT_SUCCESS;
    case CLI_ERROR:
    default:
        return EXIT_FAILURE;
    }

    install_signal_handlers();
    return driver_run(&cfg);
}
