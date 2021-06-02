#ifndef JS_H
#define JS_H

#include <stddef.h>

char *js_exts[] = {".js", NULL};

char *js_keywords[] = {
    "await", "break", "case", "catch", "continue", "debugger", "default",
    "do", "else", "eval", "export", "finally", "for", "if",
    "implements", "import", "return", "switch", "throw", "try", "while",
    "with", "yield",

    // types, variables & functions related
    "arguments|", "class|", "const|", "delete|", "enum|", "extends|", "false|",
    "function|", "in|", "instanceof|", "interface|", "let|", "new|", "null|",
    "static|", "super|", "this|", "true|", "typeof|", "var|", "void|", NULL};

#endif