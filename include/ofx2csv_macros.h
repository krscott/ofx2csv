#ifndef OFX2CSV_MACROS_H_
#define OFX2CSV_MACROS_H_

#include "ofx2csv.h" // IWYU pragma: export

#include <assert.h> // IWYU pragma: export
#include <stdio.h>  // IWYU pragma: export
#include <stdlib.h> // IWYU pragma: export

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define debugf(...)                                                            \
    do                                                                         \
    {                                                                          \
        if (ofx2csv_verbose)                                                   \
        {                                                                      \
            eprintf(__VA_ARGS__);                                              \
        }                                                                      \
    } while (0)

#define panicf(...)                                                            \
    do                                                                         \
    {                                                                          \
        eprintf(__VA_ARGS__);                                                  \
        fprintf(stderr, "\n");                                                 \
        fflush(stderr);                                                        \
        assert(false);                                                         \
        exit(134);                                                             \
    } while (0)

#define expect(cond)                                                           \
    do                                                                         \
    {                                                                          \
        if (!(cond))                                                           \
        {                                                                      \
            panicf("Failed condition: '" #cond "'\n");                         \
        }                                                                      \
    } while (0)

#endif
