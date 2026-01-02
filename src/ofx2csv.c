#include "ofx2csv.h"

#include "bits/posix2_lim.h"
#include "ktl/lib/strings.h"
#include "ktl/lib/strings.inc"
#include "ofx2csv_macros.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ktl_vec ofx2csv_rows
#include "ktl/struct/vec.inc"
#undef ktl_vec

typedef struct
{
    strview tail;
    char const *err;
} cursor;

static void cursor_get_line_and_col(
    cursor const c,
    strview const source,
    size_t *const lineno,
    size_t *const colno
)
{
    expect(c.tail.ptr);
    expect(source.ptr);
    expect(c.tail.ptr >= source.ptr);

    size_t line_count = 1;
    size_t col_count = 0;

    size_t const len = (uintptr_t)c.tail.ptr - (uintptr_t)source.ptr;
    expect(len <= source.len);
    for (size_t i = 0; i < len; ++i)
    {
        if (source.ptr[i] == '\n')
        {
            ++line_count;
            col_count = 0;
        }
        else
        {
            ++col_count;
        }
    }

    if (lineno)
    {
        *lineno = line_count;
    }
    if (colno)
    {
        *colno = col_count;
    }
}

bool ofx2csv_verbose = false;

nodiscard ofx2csv_data ofx2csv_data_init(void)
{
    return (ofx2csv_data){
        ofx2csv_rows_init(),
    };
}
void ofx2csv_data_deinit(ofx2csv_data *const data)
{
    ofx2csv_rows_deinit(&data->rows);
}

static nodiscard bool parse_remaining(cursor *const c, strview *const remaining)
{
    bool const ok = c->tail.len > 0;
    if (!ok)
    {
        c->err = "Unexpected end of file";
    }
    else
    {
        *remaining = c->tail;
        c->tail = (strview){&c->tail.ptr[c->tail.len], 0};
    }
    return ok;
}

static nodiscard bool parse_line(cursor *const c, strview *const line)
{
    return strview_split(c->tail, '\n', line, &c->tail) ||
           parse_remaining(c, line);
}

static nodiscard bool parse_metadata(cursor *const c)
{
    cursor tmp = *c;
    strview line, key, value;
    bool const ok =
        parse_line(&tmp, &line) && strview_split(line, ':', &key, &value);

    if (ok)
    {
        *c = tmp;
        debugf(
            "meta " strview_fmts " : " strview_fmts "\n",
            strview_fmtv(key),
            strview_fmtv(value)
        );
    }
    else
    {
        c->err = "Expected metadata";
    }

    return ok;
}

nodiscard bool ofx2csv_data_parse(
    ofx2csv_data *const data,
    char const *const s,
    size_t const n,
    char const *filename
)
{
    if (!filename)
    {
        filename = "<unknown>";
    }

    (void)data;
    strview const source = {s, n};

    cursor c = {.tail = source};

    while (parse_metadata(&c))
    {
    }

    bool const ok = true;

    // strview line;
    // while (c.tail.len && parse_line(&c, &line))
    // {
    //     printf("line: `" strview_fmts "`\n", strview_fmtv(line));
    // }

    if (!ok)
    {
        expect(c.err);

        size_t lineno, colno;
        cursor_get_line_and_col(c, source, &lineno, &colno);
        eprintf(
            "Parse error: %s\n  at %s:%lu:%lu\n",
            c.err,
            filename,
            lineno,
            colno
        );
    }

    return !c.err;
}
