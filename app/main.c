#include "ofx2csv.h"
#include "ofx2csv_macros.h"

#include "kcli.inc"
#include "sys/types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    ofx2csv_verbose = opts.verbose;

    debugf("Parsing file: %s\n", opts.filename);
    FILE *const fp = fopen(opts.filename, "r");
    expectf_perror(fp, "fopen");

    char *buffer = NULL;
    size_t len;
    ssize_t const bytes_read = getdelim(&buffer, &len, '\0', fp);
    expectf_perror(bytes_read >= 0, "getdelim");
    fclose(fp);

    ofx2csv_data data;
    (void)ofx2csv_data_parse(&data, buffer, (size_t)bytes_read, opts.filename);

    free(buffer);
    return 0;
}
