#ifndef PY_H
#define PY_H

#include <stddef.h>

char *py_exts[] = {".py", NULL};

char *py_keywords[] = {
    "and", "as", "assert", "async", "await", "break", "continue",
    "del", "elif", "else", "except", "finally", "for", "from",
    "if", "import", "not", "or", "pass", "raise", "return",
    "try", "while", "with", "yield",

    // types, variables & functions related
    "class|", "def|", "False|", "global|", "in|", "is|", "lambda|",
    "None|", "nonlocal|", "self|", "True|", NULL};

#endif