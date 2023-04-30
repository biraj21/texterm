#include <stdlib.h>
#include <string.h>
#include "strbuf.h"

int strb_append(StringBuffer *sbptr, char *str, size_t len)
{
    char *new_str = realloc(sbptr->str, sbptr->len + len);
    if (sbptr->len + len > 0 && new_str == NULL) {
        return -1;
    }

    memcpy(&new_str[sbptr->len], str, len);
    sbptr->str = new_str;
    sbptr->len += len;

    return 0;
}

void strb_free(StringBuffer *sbptr) {
    free(sbptr->str);
}