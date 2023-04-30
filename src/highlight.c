#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "editor.h"
#include "highlight.h"
#include "languages.h"

#define is_separator(c) (isspace(c) || c == '\0' || strchr(",.=+-/*%><!&|~()[]{}:;", c) != NULL)

extern Editor editor;

int select_syntax_hl(const char *filename)
{
    if (filename == NULL)
        return 0;

    char *ext = strrchr(filename, '.');
    if (ext == NULL)
        return 0;

    for (int i = 0; i < HLDB_ENTRIES; ++i)
    {
        SyntaxInfo *syn = &syntax_hl_db[i];
        for (int j = 0; syn->file_exts[j] != NULL; ++j)
        {
            if (strcmp(ext, syn->file_exts[j]) == 0)
            {
                editor.syntax = syn;

                for (int y = 0; y < editor.numrows; ++y)
                {
                    if (syntax_update(&editor.rows[y]) == -1)
                        return -1;
                }
                return 0;
            }
        }
    }

    return 0;
}

int syntax_update(EditorRow *row)
{
    if (row->rlen > row->hlen)
    {
        row->hlen = row->rlen;
        char *new_hl = realloc(row->hl, row->rlen);
        if (new_hl == NULL)
            return -1;

        row->hl = new_hl;
    }

    memset(row->hl, HL_DEFAULT, row->rlen);

    // we want to translate & highlight non-printable
    // characters even if the syntax is not selected
    for (size_t i = 0; i < row->rlen; ++i)
    {
        char c = row->render[i];
        if (c < 0)
        {
            row->render[i] = '?';
            row->hl[i] = HL_NON_PRINT;
        }
        if (iscntrl(c))
        {
            row->render[i] = (c >= 0 && c <= 26) ? '@' + c : '?';
            row->hl[i] = HL_NON_PRINT;
        }
    }

    if (editor.syntax == NULL)
        return 0;

    char *scs = editor.syntax->sl_comment_start;
    int scs_len = (scs != NULL) ? strlen(scs) : 0;

    char *mcs = editor.syntax->ml_comment_start;
    int mcs_len = (mcs != NULL) ? strlen(mcs) : 0;

    char *mce = editor.syntax->ml_comment_end;
    int mce_len = (mce != NULL) ? strlen(mce) : 0;

    char **keywords = editor.syntax->keywords;

    bool prev_sep = true;
    bool in_ml_comment = row->index > 0 && editor.rows[row->index - 1].is_comment_open;
    char open_char = 0;
    for (size_t i = 0; i < row->rlen; ++i)
    {
        // single line comment
        if (scs_len > 0 && !open_char && !in_ml_comment)
        {
            if (strncmp(scs, &row->render[i], scs_len) == 0)
            {
                memset(&row->hl[i], HL_COMMENT, row->rlen - i);
                break;
            }
        }

        // multi line comment
        if (mcs_len > 0 && mce_len > 0 && !open_char)
        {
            if (in_ml_comment)
            {
                row->hl[i] = HL_COMMENT;

                // checking if comment ends
                if (strncmp(mce, &row->render[i], mce_len) == 0)
                {
                    memset(&row->hl[i], HL_COMMENT, mce_len);
                    i += mce_len - 1;
                    in_ml_comment = false;
                    prev_sep = true;
                }

                continue;
            }
            else if (strncmp(mcs, &row->render[i], mcs_len) == 0)
            {
                memset(&row->hl[i], HL_COMMENT, mcs_len);
                i += mcs_len - 1;
                in_ml_comment = true;
                continue;
            }
        }

        char c = row->render[i];
        char prev_hl = (i > 0) ? row->hl[i - 1] : HL_DEFAULT;

        // strings
        if (editor.syntax->flags & HIGHLIGHT_CHARS)
        {
            if (open_char)
            {
                if (c == '\\' && i + 1 < row->rlen)
                {
                    row->hl[i++] = HL_ESC_SEQ;
                    row->hl[i] = HL_ESC_SEQ;
                    continue;
                }

                row->hl[i] = HL_CHAR;
                if (c == open_char)
                    open_char = 0;

                prev_sep = true;
                continue;
            }
            else if (c == '"' || c == '\'')
            {
                open_char = c;
                row->hl[i] = HL_CHAR;
                continue;
            }
        }

        // numbers
        if (editor.syntax->flags & HIGHLIGHT_NUMBERS)
        {
            if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) || (c == '.' && prev_hl == HL_NUMBER))
            {
                row->hl[i] = HL_NUMBER;
                prev_sep = false;
                continue;
            }
        }

        if (prev_sep)
        {
            char *kw = NULL;
            for (int k = 0; (kw = keywords[k]) != NULL; ++k)
            {
                int kwlen = strlen(kw);
                bool is_kw2 = kw[kwlen - 1] == '|';
                if (is_kw2)
                    --kwlen;

                if (editor.syntax->flags & HIGHLIGHT_IGNORE_CASE)
                {
                    if (strncasecmp(kw, &row->render[i], kwlen) == 0 && is_separator(row->render[i + kwlen]))
                    {
                        memset(&row->hl[i], (is_kw2) ? HL_KEYWORD2 : HL_KEYWORD1, kwlen);
                        i += kwlen - 1;
                        break;
                    }
                }
                else
                {
                    if (strncmp(kw, &row->render[i], kwlen) == 0 && is_separator(row->render[i + kwlen]))
                    {
                        memset(&row->hl[i], (is_kw2) ? HL_KEYWORD2 : HL_KEYWORD1, kwlen);
                        i += kwlen - 1;
                        break;
                    }
                }
            }

            // if kw != NULL, it means that we found a keyword
            // & broke out of the loop after highlighting it
            if (kw != NULL)
            {
                prev_sep = false;
                continue;
            }
        }

        if (editor.syntax->flags & HIGHLIGHT_C_PREPROCS)
        {
            if ((i == 0 || isspace(row->render[i - 1])) && c == '#')
            {
                char *preproc = NULL;
                for (int j = 0; (preproc = c_preprocs[j]) != NULL; ++j)
                {
                    int preproc_len = strlen(preproc);
                    char a;
                    if (strncmp(preproc, &row->render[i + 1], preproc_len) == 0 && (isspace(a = row->render[i + preproc_len + 1]) || a == '<' || a == '"' || a == '\0'))
                    {
                        memset(&row->hl[i], HL_PREPROC, preproc_len + 1);
                        i += preproc_len;

                        if (strcmp(preproc, "include") == 0)
                        {
                            char open_header = 0;
                            for (size_t s = i + 1; s < row->rlen; ++s)
                            {
                                if (isspace(row->render[s]))
                                    continue;
                                else if (open_header)
                                {
                                    row->hl[s] = HL_HEADER;
                                    if ((open_header == '"' && row->render[s] == '"') || (open_header == '<' && row->render[s] == '>'))
                                    {
                                        open_header = 0;
                                        break;
                                    }
                                }
                                else if (row->render[s] == '"' || row->render[s] == '<')
                                {
                                    open_header = row->render[s];
                                    row->hl[s] = HL_HEADER;
                                }
                                else
                                    break;
                            }
                        }

                        break;
                    }
                }

                // if preproc != NULL, it means that we found a keyword
                // & broke out of the loop after highlighting it
                if (preproc != NULL)
                {
                    prev_sep = false;
                    continue;
                }
            }
        }

        prev_sep = is_separator(c);
    }

    bool comment_state_changed = row->is_comment_open != in_ml_comment;
    row->is_comment_open = in_ml_comment;
    if (comment_state_changed && row->index + 1 < editor.numrows)
        syntax_update(&editor.rows[row->index + 1]);

    return 0;
}

int get_color(char hl)
{
    switch (hl)
    {
    case HL_HEADER:
    case HL_CHAR:
        return 214;
    case HL_ESC_SEQ:
        return 11;
    case HL_COMMENT:
        return 35;
    case HL_NUMBER:
        return 228;
    case HL_PREPROC:
        return 207;
    case HL_KEYWORD1:
        return 207;
    case HL_KEYWORD2:
        return 69;
    case HL_NON_PRINT:
        return 32;
    default:
        return -1;
    }
}

void highlight(EditorRow *row, int index, int len, StringBuffer *sbptr)
{
    char curr_hl = -1;
    for (int i = 0; i < len; ++i)
    {
        char hl = row->hl[i];
        if (curr_hl != hl)
        {
            curr_hl = hl;

            char color_seq[32];
            if (hl == HL_DEFAULT)
                snprintf(color_seq, sizeof(color_seq), "\x1b[48;2;17;17;17m\x1b[39m");
            else if (hl == HL_MATCH)
                snprintf(color_seq, sizeof(color_seq), "\x1b[48;5;173m\x1b[38;2;0;0;0m");
            else
                snprintf(color_seq, sizeof(color_seq), "\x1b[48;2;17;17;17m\x1b[38;5;%dm", get_color(hl));

            strb_append(sbptr, color_seq, strlen(color_seq));
        }

        strb_append(sbptr, &row->render[index++], 1);
    }
}

static char *saved_hl = NULL;
static EditorRow *saved_hl_row = NULL;

void highlight_match_set(EditorRow *row, int index, int len)
{
    saved_hl = malloc(row->rlen);
    if (saved_hl == NULL)
        return;

    memcpy(saved_hl, row->hl, row->rlen);
    saved_hl_row = row;
    memset(&row->hl[index], HL_MATCH, len);
}

void highlight_match_reset()
{
    if (saved_hl == NULL)
        return;

    memcpy(saved_hl_row->hl, saved_hl, saved_hl_row->rlen);
    free(saved_hl);
    saved_hl = NULL;
    saved_hl_row = NULL;
}
