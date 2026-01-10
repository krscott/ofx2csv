#include "ofx2csv.h"

#include "ktl/lib/integers.h"
#include "ktl/lib/strings.h"
#include "ktl/lib/strings.inc"
#include "ofx2csv_macros.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ktl_vec ofx2csv_rows
#include "ktl/struct/vec.inc"
#undef ktl_vec

typedef struct
{
    strview tail;
    char const *err;
} cursor;

#define TAG_OFX strview_const("OFX")
#define TAG_STMTTRN strview_const("STMTTRN")
#define TAG_ACCTID strview_const("ACCTID")
#define TAG_DTPOSTED strview_const("DTPOSTED")
#define TAG_TRNAMT strview_const("TRNAMT")
#define TAG_NAME strview_const("NAME")
#define TAG_MEMO strview_const("MEMO")
#define TAG_BALAMT strview_const("BALAMT")

#define money_fmts i64_fmts "." i64_fmts
#define money_fmtv(x) ((x) / 100), (i64_abs(x) % 100)

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

static void parse_skip_whitespace(cursor *const c)
{
    c->tail = strview_trim_start(c->tail);
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

static nodiscard bool
parse_metadata(cursor *const c, strview *const key, strview *const value)
{
    cursor tmp = *c;
    strview line;
    bool const ok =
        parse_line(&tmp, &line) && strview_split(line, ':', key, value);

    if (ok)
    {
        *c = tmp;
    }
    else
    {
        c->err = "Expected metadata";
    }

    return ok;
}

typedef enum
{
    TAG_VALUE,
    TAG_START,
    TAG_END,
} tag_kind;

static nodiscard bool parse_tag(
    cursor *const c,
    tag_kind *const kind,
    strview *const name,
    strview *const value
)
{
    cursor tmp = *c;
    strview line;

    parse_skip_whitespace(&tmp);
    bool const ok = parse_line(&tmp, &line) &&
                    strview_split(line, '<', NULL, &line) &&
                    strview_split(line, '>', name, value);
    if (ok)
    {
        *c = tmp;

        *value = strview_trim_end(*value);
        if (value->len > 0)
        {
            *kind = TAG_VALUE;
        }
        else if (name->ptr[0] == '/')
        {
            *kind = TAG_END;
            expect(strview_split_first(*name, NULL, name));
        }
        else
        {
            *kind = TAG_START;
        }
    }
    else
    {
        c->err = "Expected tag";
    }

    return ok;
}

static nodiscard bool get_decimal(strview sv, i64 *decimal)
{
    bool ok = sv.len > 0;
    i64 out = 0;

    for (size_t i = 0; ok && i < sv.len; ++i)
    {
        char const ch = sv.ptr[i];
        if ('0' <= ch && ch <= '9')
        {
            out = out * 10 + (ch - '0');
        }
        else
        {
            ok = false;
        }
    }

    if (ok)
    {
        *decimal = out;
    }

    return ok;
}

static nodiscard bool
get_date(strview sv, u16 *const year, u8 *const month, u8 *const day)
{
    bool ok = sv.len >= 8;
    if (ok)
    {
        strview year_sv, month_sv, day_sv;
        strview_split_at(sv, 4, &year_sv, &sv);
        strview_split_at(sv, 2, &month_sv, &sv);
        strview_split_at(sv, 2, &day_sv, &sv);

        i64 year_i64 = 0;
        i64 month_i64 = 0;
        i64 day_i64 = 0;
        ok = get_decimal(year_sv, &year_i64)      //
             && get_decimal(month_sv, &month_i64) //
             && get_decimal(day_sv, &day_i64);

        if (ok)
        {
            *year = (u16)year_i64;
            *month = (u8)month_i64;
            *day = (u8)day_i64;
        }
    }

    return ok;
}

static nodiscard bool get_currency_amount(strview sv, i64 *const amount_cents)
{
    bool const neg = strview_starts_with_cstr(sv, "-");
    if (neg)
    {
        expect(strview_split_first(sv, NULL, &sv));
    }

    strview dollars_sv, cents_sv;
    i64 dollars = 0;
    i64 cents = 0;
    bool const ok = strview_split(sv, '.', &dollars_sv, &cents_sv) //
                    && (cents_sv.len == 2)                         //
                    && get_decimal(dollars_sv, &dollars)           //
                    && get_decimal(cents_sv, &cents);

    if (ok)
    {
        i64 out = dollars * 100 + cents;
        *amount_cents = neg ? -out : out;
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

    strview const source = {s, n};

    cursor c = {.tail = source};

    bool ok = true;

    strview key, value;

    while (parse_metadata(&c, &key, &value))
    {
        debugf(
            "meta " strview_fmts " : " strview_fmts "\n",
            strview_fmtv(key),
            strview_fmtv(value)
        );
    }

    strview account = strview_const("?");
    ofx2csv_row row = {0};
    bool row_already_added = false;

    size_t const first_row = data->rows.len;

    bool found_amount = false;
    i64 running_balance_cents = 0;

    tag_kind kind;
    while (ok && (ok = parse_tag(&c, &kind, &key, &value)))
    {
        switch (kind)
        {
            case TAG_VALUE:
                debugf(
                    "tag " strview_fmts " : " strview_fmts "\n",
                    strview_fmtv(key),
                    strview_fmtv(value)
                );

                if (strview_eq(key, TAG_ACCTID))
                {
                    account = value;
                }
                else if (strview_eq(key, TAG_DTPOSTED))
                {
                    ok = get_date(value, &row.year, &row.month, &row.day);
                    if (!ok)
                    {
                        c.err = "Failed to parse date";
                    }
                }
                else if (strview_eq(key, TAG_TRNAMT))
                {
                    ok = get_currency_amount(value, &row.amount_cents);
                    if (!ok)
                    {
                        c.err = "Failed to parse dollar amount";
                    }
                }
                else if (strview_eq(key, TAG_NAME))
                {
                    row.name = value;
                }
                else if (strview_eq(key, TAG_MEMO))
                {
                    row.memo = value;
                }
                else if (strview_eq(key, TAG_BALAMT))
                {
                    // TODO: Use only specific BALAMT (LEDGERBAL or AVAILBAL)
                    found_amount = true;
                    ok = get_currency_amount(value, &running_balance_cents);
                    if (!ok)
                    {
                        c.err = "Failed to parse dollar amount";
                    }
                }

                break;
            case TAG_START:
                debugf("tag " strview_fmts " START\n", strview_fmtv(key));
                if (strview_eq(key, TAG_STMTTRN))
                {
                    memset(&row, 0, sizeof(row));
                    row_already_added = false;
                }
                break;
            case TAG_END:
                debugf("tag " strview_fmts " END\n", strview_fmtv(key));
                if (strview_eq(key, TAG_STMTTRN))
                {
                    if (row_already_added)
                    {
                        ok = false;
                        c.err = "Expected <STMTTRN>";
                    }
                    else
                    {
                        row.account = account;
                        ofx2csv_rows_push(&data->rows, row);
                        row_already_added = true;
                    }
                }
                else if (strview_eq(key, TAG_OFX))
                {
                    goto exit_tag_parsing;
                }
                break;
        }
    }
exit_tag_parsing:

    if (ok && !found_amount)
    {
        ok = false;
        c.err = "Expected <BALAMT>";
    }

    if (ok)
    {
        for (size_t i = data->rows.len; i > first_row; --i)
        {
            ofx2csv_row *const row_ = &data->rows.ptr[i - 1];
            row_->balance_cents = running_balance_cents;
            running_balance_cents -= row_->amount_cents;
        }
    }
    else
    {
        expect(c.err);

        size_t lineno, colno;
        cursor_get_line_and_col(c, source, &lineno, &colno);
        eprintf(
            "Parse error: %s\n  at %s:%zu:%zu\n",
            c.err,
            filename,
            lineno,
            colno
        );
    }

    return ok;
}

static void write_escaped_string(strview const sv, FILE *const stream)
{
    fputc('"', stream);

    for (size_t i = 0; i < sv.len; ++i)
    {
        char const ch = sv.ptr[i];
        if (ch == '"' || ch == '\\')
        {
            fputc('\\', stream);
        }
        fputc(ch, stream);
    }

    fputc('"', stream);
}

void ofx2csv_data_write_csv(ofx2csv_data const *const data, FILE *const stream)
{
    fprintf(stream, "Account,Date,Name,Memo,Amount,Balance\n");

    for (size_t i = 0; i < data->rows.len; ++i)
    {
        ofx2csv_row const row = data->rows.ptr[i];
        write_escaped_string(row.account, stream);
        fputc(',', stream);
        fprintf(stream, "%04u-%02u-%02u", row.year, row.month, row.day);
        fputc(',', stream);
        write_escaped_string(row.name, stream);
        fputc(',', stream);
        write_escaped_string(row.memo, stream);
        fputc(',', stream);
        fprintf(stream, money_fmts, money_fmtv(row.amount_cents));
        fputc(',', stream);
        fprintf(stream, money_fmts, money_fmtv(row.balance_cents));
        fputc('\n', stream);
    }

    fflush(stream);
}
