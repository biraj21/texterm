#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <stddef.h>
#include "./highlight.h"

#include "./languages/c.h"
#include "./languages/js.h"
#include "./languages/py.h"
#include "./languages/sql.h"

// Database of languages
SyntaxInfo syntax_hl_db[] = {
    {
        "c",
        c_exts,
        c_keywords,
        "//",
        "/*",
        "*/",
        HIGHLIGHT_NUMBERS | HIGHLIGHT_CHARS | HIGHLIGHT_C_PREPROCS,
    },
    {
        "js",
        js_exts,
        js_keywords,
        "//",
        "/*",
        "*/",
        HIGHLIGHT_NUMBERS | HIGHLIGHT_CHARS,
    },
    {
        "py",
        py_exts,
        py_keywords,
        "#",
        "\"\"\"",
        "\"\"\"",
        HIGHLIGHT_NUMBERS | HIGHLIGHT_CHARS,
    },
    {
        "sql",
        sql_exts,
        sql_commands,
        "--",
        "/*",
        "*/",
        HIGHLIGHT_NUMBERS | HIGHLIGHT_CHARS | HIGHLIGHT_IGNORE_CASE,
    },
};

const int HLDB_ENTRIES = sizeof(syntax_hl_db) / sizeof(SyntaxInfo);

#endif