#include "cstart.h"

#include "ktest.inc"
#include <stdlib.h>
#include <string.h>

KTEST_MAIN
{
    KTEST(t_greet_name)
    {
        char *greeting = cstart_create_greeting("Kris");
        ASSERT_TRUE(greeting);

        ASSERT_TRUE(0 == strcmp(greeting, "Hello, Kris!"));

        free(greeting);
    }

    KTEST(t_greet_null)
    {
        char *greeting = cstart_create_greeting(NULL);
        ASSERT_TRUE(greeting);

        ASSERT_TRUE(0 == strcmp(greeting, "Hello, World!"));

        free(greeting);
    }
}
