#include "ofx2csv.h"
#include "ofx2csv_macros.h"

#include "kcli.inc"
#include "ktl/lib/io.h"
#include "ktl/lib/io.inc"
#include "ktl/lib/strings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

    strbuf input_buf = strbuf_init();
    expectf_perror(
        strbuf_append_stream(&input_buf, input_file),
        "File read error"
    );
    fclose(input_file);

    ofx2csv_data data = ofx2csv_data_init();
    bool ok = ofx2csv_data_parse(
        &data,
        input_buf.ptr,
        input_buf.len,
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

    strbuf_deinit(&input_buf);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
