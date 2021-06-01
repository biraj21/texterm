#ifndef ROWS_H
#define ROWS_H

#include <stdbool.h>

typedef struct
{
    int index;
    size_t len;
    char *text;
    size_t rlen;
    char *render;
    size_t hlen;
    unsigned char *hl; // colors to be used for each char in line
    bool is_comment_open;
} EditorRow;

int row_cx_to_rx(EditorRow *row, int cx);
int row_update(EditorRow *row);
int row_insert_char(EditorRow *row, int at, char c);
int row_delete_char(EditorRow *row, int at);
int row_append_string(EditorRow *row, char *str, size_t len);
void free_row(EditorRow *row);

#endif
