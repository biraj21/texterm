#ifndef C_H
#define C_H

#include <stddef.h>

char *c_exts[] = {".c", ".h", ".cpp", NULL};

char *c_keywords[] = {
    "auto", "break", "case", "continue", "do", "default", "else",
    "extern", "for", "if", "goto", "register", "return", "sizeof",
    "switch", "volatile", "while", "restrict",

    /* C11 */
    "_Alignas",
    "_Alignof",
    "_Atomic",
    "_Generic",
    "_Noreturn",
    "_Static_assert",
    "_Thread_local",

    /* C99 */
    "_Bool",
    "_Complex",
    "_Imaginary,

    // types, variables & functions related
    "char|", "const|", "double|", "enum|", "float| ", "int|", "long|",
    "short|", "signed|", "static|", "struct|", "typedef|", "union|", "unsigned|",
    "void|", NULL};

char *c_preprocs[] = {
    "define", "defined", "elif", "else", "endif", "error", "if", "ifdef",
    "ifndef", "include", "undef", "pragma", NULL};

#endif
