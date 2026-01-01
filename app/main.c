#include "cstart.h"

#include "kcli.inc"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct opts
{
    char const *name;
    bool verbose;
};

static struct opts opts_parse(int const argc, char const *const *const argv)
{
    struct opts opts = {0};

    KCLI_PARSE(
        argc,
        argv,
        {
            .pos_name = "name",
            .ptr_str = &opts.name,
            .optional = true,
            .help = "Name to greet",
        },
        {
            .short_name = 'v',
            .long_name = "verbose",
            .ptr_flag = &opts.verbose,
            .help = "Enable extra logging",
        },
    );

    return opts;
}

int main(int const argc, char const *const *const argv)
{
    struct opts opts = opts_parse(argc, argv);

    if (opts.verbose)
    {
        fprintf(stderr, "c-start: Creating greeting...\n");
    }

    char *greeting = cstart_create_greeting(opts.name);

    printf("%s\n", greeting);

    free(greeting);

    return greeting ? 0 : 1;
}
