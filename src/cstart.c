#include "cstart.h"
#include "ktl/lib/strings.inc"
#include <assert.h>
#include <stddef.h>

// Create a greeting string.
// Allocates - client code is responsible for freeing `name`
char *cstart_create_greeting(char const *const name)
{
    struct strbuf s = {0};

    strbuf_append_terminated(&s, "Hello, ");
    strbuf_append_terminated(&s, name ? name : "World");
    strbuf_append_terminated(&s, "!");

    return s.ptr;
}
