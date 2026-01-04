#ifndef OFX2CSV_MACROS_H_
#define OFX2CSV_MACROS_H_

#include "ofx2csv.h" // IWYU pragma: export

#include <assert.h> // IWYU pragma: export
#include <errno.h>  // IWYU pragma: export
#include <stdio.h>  // IWYU pragma: export
#include <stdlib.h> // IWYU pragma: export

#define nodiscard ofx2csv_nodiscard

#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))

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
        exit(EXIT_FAILURE);                                                    \
    } while (0)

#define expectf(cond, ...)                                                     \
    do                                                                         \
    {                                                                          \
        if (!(cond))                                                           \
        {                                                                      \
            panicf(__VA_ARGS__);                                               \
        }                                                                      \
    } while (0)

#define expectf_perror(cond, ...)                                              \
    do                                                                         \
    {                                                                          \
        if (!(cond))                                                           \
        {                                                                      \
            eprintf(__VA_ARGS__);                                              \
            panicf(": %s", strerror(errno));                                   \
        }                                                                      \
    } while (0)

#define expect(cond) expectf((cond), "Failed condition: '" #cond "'")

#endif
