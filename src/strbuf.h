#ifndef STRBUF_H
#define STRBUF_H

#include <stddef.h>

#define STRBUF_INIT \
    {               \
        0, NULL     \
    }

typedef struct strbuf
{
    size_t len;
    char *str;
} StringBuffer;

int strb_append(StringBuffer *sbptr, char *str, size_t len);
void strb_free(StringBuffer *sbptr);

#endif