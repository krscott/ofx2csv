#include "ofx2csv.h"
#include "ofx2csv_macros.h"

#include "kcli.inc"
#include "ktl/lib/io.h"
#include "ktl/lib/io.inc"
#include "ktl/lib/strings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char const *input_filenames[16];
    size_t input_filenames_count;
    char const *output_filename;
    bool verbose;
} cliopts;

#define DEFAULT_OUTPUT_FILENAME "ofx2csv-output.csv"

static cliopts cliopts_parse(int const argc, char const *const *const argv)
{
    cliopts opts = {
        .output_filename = DEFAULT_OUTPUT_FILENAME,
    };

    KCLI_PARSE(
        argc,
        argv,
        {
            .name = "file",
            .ptr_str = opts.input_filenames,
            .ptr_nargs = &opts.input_filenames_count,
            .nargs_max = countof(opts.input_filenames),
            .help = "File to parse",
        },
        {
            .short_name = 'o',
            .long_name = "output",
            .name = "file",
            .ptr_str = &opts.output_filename,
            .help = "Output file (defaults to '" DEFAULT_OUTPUT_FILENAME "')",
        },
        {
            .short_name = 'v',
            .long_name = "verbose",
            .ptr_flag = &opts.verbose,
            .help = "Enable extra logging",
        },
    );

    if (0 == strcmp(opts.output_filename, "-"))
    {
        // Use stdout
        opts.output_filename = NULL;
    }

    return opts;
}

int main(int const argc, char const *const *const argv)
{
    cliopts opts = cliopts_parse(argc, argv);
    ofx2csv_verbose = opts.verbose;

    strbuf input_buf = strbuf_init();
    ofx2csv_data data = ofx2csv_data_init();

    bool ok = true;

    for (size_t i = 0; ok && i < opts.input_filenames_count; ++i)
    {
        char const *const input_filename = opts.input_filenames[i];
        debugf("Parsing file: %s\n", input_filename);

        {
            FILE *const input_file = fopen(input_filename, "r");
            expectf_perror(input_file, "fopen");

            strbuf_clear(&input_buf);
            expectf_perror(
                strbuf_append_stream(&input_buf, input_file),
                "File read error"
            );

            fclose(input_file);
        }

        ok = ofx2csv_data_parse(
            &data,
            input_buf.ptr,
            input_buf.len,
            input_filename
        );
    }

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
