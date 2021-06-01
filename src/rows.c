#include <stdlib.h>
#include <string.h>
#include "../includes/editor.h"
#include "../includes/highlight.h"
#include "../includes/rows.h"

extern Editor e;

int row_cx_to_rx(EditorRow *row, int cx)
{
    int rx = 0;
    for (int i = 0; i < cx; ++i)
    {
        if (row->text[i] == '\t')
            rx += e.tab_stop - rx % e.tab_stop;
        else
            ++rx;
    }

    return rx;
}

int row_update(EditorRow *row)
{
    int tabs = 0;
    for (int i = 0; i < row->len; ++i)
    {
        if (row->text[i] == '\t')
            ++tabs;
    }

    char *new_render = malloc(row->len + (e.tab_stop - 1) * tabs + 1);
    if (new_render == NULL)
        return -1;

    free(row->render);
    row->render = new_render;

    int index = 0;
    for (int i = 0; i < row->len; ++i)
    {
        if (row->text[i] == '\t')
        {
            for (int x = index % e.tab_stop; x < e.tab_stop; ++x)
                row->render[index++] = ' ';
        }
        else
            row->render[index++] = row->text[i];
    }

    row->render[index] = '\0';
    row->rlen = index;

    return syntax_update(row);
}

int row_insert_char(EditorRow *row, int at, char c)
{
    if (at < 0 || at > row->len)
        return 0;

    char *new_str = realloc(row->text, row->len + 2);
    if (new_str == NULL)
        return -1;

    row->text = new_str;
    memmove(&row->text[at + 1], &row->text[at], row->len - at + 1);
    row->text[at] = c;
    ++row->len;

    return row_update(row);
}

int row_delete_char(EditorRow *row, int at)
{
    if (at < 0 || at >= row->len)
        return 0;

    memmove(&row->text[at], &row->text[at + 1], row->len - at);
    --row->len;
    return row_update(row);
}

int row_append_string(EditorRow *row, char *str, size_t len)
{
    char *new_text = realloc(row->text, row->len + len + 1);
    if (new_text == NULL)
        return -1;

    memcpy(&new_text[row->len], str, len);
    row->text = new_text;
    row->len += len;
    row->text[row->len] = '\0';

    return row_update(row);
}

void free_row(EditorRow *row)
{
    free(row->text);
    free(row->render);
    free(row->hl);
}
