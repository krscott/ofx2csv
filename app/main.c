#include "ofx2csv.h"

#include "kcli.inc"
#include <stdbool.h>

typedef struct
{
    char const *filename;
    bool verbose;
} cliopts;

static cliopts cliopts_parse(int const argc, char const *const *const argv)
{
    cliopts opts = {0};

    KCLI_PARSE(
        argc,
        argv,
        {
            .pos_name = "file",
            .ptr_str = &opts.filename,
            .optional = true,
            .help = "File to parse",
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
    cliopts opts = cliopts_parse(argc, argv);
    verbose = opts.verbose;

    debugf("Parsing file: %s\n", opts.filename);

    panicf("todo");

    return 0;
}
