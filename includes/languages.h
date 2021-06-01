#include <stddef.h>
#include "./highlight.h"

/***  C & C++  ***/
char *c_exts[] = {".c", ".h", ".cpp", NULL};
char *c_keywords[] = {
    "auto", "break", "case", "continue", "do", "default", "else", "extern", "for", "if", "goto", "register", "return", " sizeof ", " switch ", "volatile", "while",
    // type related
    "char|", "const|", "double|", "enum|", "float| ", "int|", "long|", "short|", "signed|", "static|", "struct|", "typedef|", "union|", "unsigned|", "void|", NULL};
char *c_preprocs[] = {"define", "elif", "else", "endif", "error", "if", "ifdef", "ifndef", "include", "undef", "pragma", NULL};

/***  JavaScript  ***/
char *js_exts[] = {".js", NULL};
char *js_keywords[] = {
    "await", "break", "case", "catch", "continue", "debugger", "default", "do", "else", "eval", "export", "finally", "for", "if", "implements", "import", "return", "switch", "throw", "try", "while", "with", "yield",
    // type/variable related
    "arguments|", "class|", "const|", "delete|", "enum|", "extends|", "in|", "instanceof|", "interface|", "let|", "new|", "null|", "false|", "function|", "static|", "super|", "this|", "true|", "typeof|", "var|", "void|", NULL};

/*** SQL ***/
char *sql_exts[] = {".sql", NULL};
char *sql_commands[] = {
    "SELECT","UPDATE","CREATE",
    "DROP","DELETE","INSERT","INTO","ALTER",
    "DATABASE","TABLE",
    "GRANT","REVOKE",
    "COMMIT","ROLLBACK",
    "INDEX",
    "MIN","MAX","AVG","COUNT","SUM",
    "BACKUP","TO","DISK","WITH","DIFFERENTIAL","BETWEEN",

    "WHERE","LIKE","IN","AS","HAVING",
    "UNION","JOIN","GROUP","LEFT","RIGHT","INNER",
    "NOT","NULL","PRIMARY","KEY",
    "UNIQUE","DEFAULT","AUTO_INCREMENT","TIMESTAMP",
    
    // DATA TYPES
    "VARCHAR|","INT|","TINYINT|","BIGINT|","FLOAT|","REAL|",
    "DATE|","TIME|","DATETIME|","NCHAR|","NVARCHAR|","NTEXT|",
    "BINARY|","VARBINARY|",
    "CLOB|","BLOB|","XML|","CURSOR|","TABLE|",

    NULL
};



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
        "sql",
        sql_exts,
        sql_commands,
        "--",
        "/*",
        "*/",       
        HIGHLIGHT_NUMBERS | HIGHLIGHT_CHARS ,
    }
};

const int HLDB_ENTRIES = sizeof(syntax_hl_db) / sizeof(SyntaxInfo);
