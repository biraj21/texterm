#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "rows.h"
#include "strbuf.h"

typedef struct {
    char *file_type;
    char **file_exts; // file extensions
    char **keywords;
    char *sl_comment_start; // start of single-line comment
    char *ml_comment_start; // start of multi-line comment
    char *ml_comment_end;   // end of multi-line comment
    int flags;              // for choosing what to highlight
} SyntaxInfo;

enum Highlight {
    // code highlighting
    HL_DEFAULT = -50,
    HL_HEADER,
    HL_CHAR,
    HL_ESC_SEQ,
    HL_COMMENT,
    HL_NUMBER,
    HL_PREPROC,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_FUNC,

    HL_MATCH,     // search results highlighting
    HL_NON_PRINT, // for non printable chars
};

enum SyntaxFlags
{
    HIGHLIGHT_NUMBERS = 1,
    HIGHLIGHT_CHARS = 1 << 1,
    HIGHLIGHT_C_PREPROCS = 1 << 2,
    HIGHLIGHT_IGNORE_CASE = 1 << 3,
    HIGHLIGHT_FUNCTIONS = 1 << 4
};

int select_syntax_hl(const char *filename);
int syntax_update(EditorRow *row);
void highlight(EditorRow *row, int index, int len, StringBuffer *sbptr);
void highlight_match_set(EditorRow *row, int index, int len);
void highlight_match_reset();

#endif
