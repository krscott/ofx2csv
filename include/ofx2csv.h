#ifndef OFX2CSV_H_
#define OFX2CSV_H_

#include "ktl/lib/strings.h"
#include "ktl/prelude.h"
#include <stdbool.h>
#include <stdio.h>

#if defined(__GNUC__) || defined(__clang__)
#define ofx2csv_nodiscard __attribute__((warn_unused_result))
#else
#define ofx2csv_nodiscard
#endif

typedef struct
{
    strview account;
    strview name;
    strview memo;
    i64 amount_cents;
    u16 year;
    u8 month;
    u8 day;
} ofx2csv_row;

#define ofx2csv_rows__type ofx2csv_row
#define ofx2csv_rows__infallible_allocator true
#define ktl_vec ofx2csv_rows
#include "ktl/struct/vec.h"
#undef ktl_vec

typedef struct
{
    ofx2csv_rows rows;
} ofx2csv_data;

ofx2csv_nodiscard ofx2csv_data ofx2csv_data_init(void);
void ofx2csv_data_deinit(ofx2csv_data *data);

ofx2csv_nodiscard bool ofx2csv_data_parse(
    ofx2csv_data *data, char const *s, size_t n, char const *filename
);
void ofx2csv_data_write_csv(ofx2csv_data const *data, FILE *stream);

extern bool ofx2csv_verbose;

#endif
