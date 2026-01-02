#ifndef OFX2CSV_H_
#define OFX2CSV_H_

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>  // IWYU pragma: export
#include <stdlib.h> // IWYU pragma: export

char *ofx2csv_create_greeting(char const *name);

extern bool verbose;

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define debugf(...)                                                            \
    do                                                                         \
    {                                                                          \
        if (verbose)                                                           \
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
