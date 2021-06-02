#ifndef C_H
#define C_H

#include <stddef.h>

char *c_exts[] = {".c", ".h", ".cpp", NULL};

char *c_keywords[] = {
    "auto", "break", "case", "continue", "do", "default", "else",
    "extern", "for", "if", "goto", "register", "return", "sizeof",
    "switch", "volatile", "while",

    // types, variables & functions related
    "char|", "const|", "double|", "enum|", "float| ", "int|", "long|",
    "short|", "signed|", "static|", "struct|", "typedef|", "union|", "unsigned|",
    "void|", NULL};

char *c_preprocs[] = {
    "define", "elif", "else", "endif", "error", "if", "ifdef",
    "ifndef", "include", "undef", "pragma", NULL};

#endif