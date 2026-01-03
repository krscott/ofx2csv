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
    char const *input_filename;
    char const *output_filename;
    char const *account_name;
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
            .ptr_str = &opts.input_filename,
            .help = "File to parse",
        },
        {
            .short_name = 'o',
            .long_name = "output",
            .ptr_str = &opts.output_filename,
            .help = "Output file (defaults to stdout)",
        },
        {
            .short_name = 'a',
            .long_name = "account",
            .ptr_str = &opts.account_name,
            .help = "Account name (defaults to Account ID)",
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

    debugf("Parsing file: %s\n", opts.input_filename);
    FILE *const input_file = fopen(opts.input_filename, "r");
    expectf_perror(input_file, "fopen");

    char *buffer = NULL;
    size_t len;
    ssize_t const bytes_read = getdelim(&buffer, &len, '\0', input_file);
    expectf_perror(bytes_read >= 0, "getdelim");
    fclose(input_file);

    ofx2csv_data data = ofx2csv_data_init();
    bool ok = ofx2csv_data_parse(
        &data,
        buffer,
        (size_t)bytes_read,
        opts.input_filename,
        opts.account_name
    );
    if (ok)
    {
        if (opts.output_filename)
        {
            debugf("Output file: %s\n", opts.output_filename);
            FILE *const output_file = fopen(opts.output_filename, "w");
            expectf_perror(output_file, "fopen");
            ofx2csv_data_write_csv(&data, output_file);
        }
        else
        {
            ofx2csv_data_write_csv(&data, stdout);
        }
    }
    ofx2csv_data_deinit(&data);

    free(buffer);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
