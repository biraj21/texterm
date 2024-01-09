#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "editor.h"
#include "strbuf.h"
#include "term.h"
#include "winsize.h"

Editor editor;
bool show_editor_cursor = true;

void editor_die(char *msg) {
    if (entered_alt_screen) {
        term_exit_alt_screen();
    }

    if (raw_mode_enabled) {
        term_disable_raw_mode();
    }

    fputs("texterm: ", stderr);
    if (errno == 0) {
        fputs(msg, stderr);
        putchar('\n');
    } else {
        perror(msg);
    }

    exit(EXIT_FAILURE);
}

void editor_init(void) {
    editor.cx = editor.cy = 0;
    editor.rx = 0;
    editor.rowoff = editor.coloff = 0;
    editor.numrows = 0;
    editor.rows = NULL;
    editor.filename = NULL;
    editor.dirty = false;
    editor.msg[0] = '\0';
    editor.msg_time = 0;
    editor.tab_stop = 4;
    editor.syntax = NULL;

    if (get_win_size(&editor.winrows, &editor.wincols) == -1) {
        editor_die("editor_init -> get_win_size");
    }

    // subtracting 2 cuz last 2nd row is status bar & last row is for prompt & message
    editor.winrows -= 2;
}

void editor_scroll(void) {
    editor.rx = 0;
    if (editor.cy < editor.numrows) {
        editor.rx = row_cx_to_rx(&editor.rows[editor.cy], editor.cx);
    }

    if (editor.cy < editor.rowoff) {
        editor.rowoff = editor.cy;
    } else if (editor.cy >= editor.rowoff + editor.winrows) {
        editor.rowoff = editor.cy + 1 - editor.winrows;
    }

    if (editor.rx < editor.coloff)  {
        editor.coloff = editor.rx;
    } else if (editor.rx >= editor.coloff + editor.wincols) {
        editor.coloff = editor.rx + 1 - editor.wincols;
    }
}

void editor_draw_rows(StringBuffer *sbptr) {
    for (int y = 0; y < editor.winrows; ++y) {
        int iy = y + editor.rowoff, textlen = 0;
        if (iy < editor.numrows) {
            textlen = editor.rows[iy].rlen - editor.coloff;
            if (textlen > editor.wincols)
                textlen = editor.wincols;

            if (textlen > 0)
            {
                highlight(&editor.rows[iy], editor.coloff, textlen, sbptr);
                if (editor.rows[iy].render[editor.coloff + textlen - 1] != HL_DEFAULT) {
                    strb_append(sbptr, "\x1b[48;2;17;17;17m\x1b[39m", 21);
                }
            }
        } else if (editor.numrows == 0 && y == editor.winrows / 2 - 1) {
            strb_append(sbptr, "~", 1);

            char ver_info[] = "Version " VERSION;
            int verlen = sizeof(ver_info) - 1;

            if (editor.wincols < verlen + 10)
                continue;

            int namelen = sizeof(NAME) - 1;
            int padlen = (editor.wincols - namelen) / 2;

            for (int i = 1; i < padlen; ++i) {
                strb_append(sbptr, " ", 1);
            }

            strb_append(sbptr, NAME, namelen);
            strb_append(sbptr, "\x1b[K\r\n~", 6);

            ++y;

            padlen = (editor.wincols - verlen) / 2;
            for (int i = 1; i < padlen; ++i) {
                strb_append(sbptr, " ", 1);
            }

            strb_append(sbptr, ver_info, verlen);
        } else {
            strb_append(sbptr, "~", 1);
        }

        strb_append(sbptr, "\x1b[K", 3);
        strb_append(sbptr, "\r\n", 2);
    }
}

void editor_status_bar(StringBuffer *sbptr) {
    strb_append(sbptr, "\x1b[48;2;221;221;221m\x1b[38;2;0;0;0m", 32);

    int namelen = 10;
    if (editor.filename == NULL) {
        strb_append(sbptr, "[new file]", namelen);
    } else {
        namelen = strlen(editor.filename);
        strb_append(sbptr, editor.filename, namelen);
    }

    if (editor.dirty) {
        strb_append(sbptr, "*", 1);
        ++namelen;
    }

    char cur_info[20];
    snprintf(cur_info, sizeof(cur_info), "Line %hu, Col %hu", editor.cy + 1, editor.rx + 1);

    int curlen = strlen(cur_info);
    for (int i = namelen; i < editor.wincols - curlen; ++i) {
        strb_append(sbptr, " ", 1);
    }

    strb_append(sbptr, cur_info, curlen);
    strb_append(sbptr, "\x1b[m\r\n", 5);
}

void editor_set_message(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vsnprintf(editor.msg, sizeof(editor.msg), msg, args);
    va_end(args);
    editor.msg_time = time(NULL);
}

void editor_message_bar(StringBuffer *sbptr) {
    char pos[16];
    snprintf(pos, sizeof(pos), "\x1b[%d;1H", editor.winrows + 2);

    strb_append(sbptr, pos, strlen(pos));
    strb_append(sbptr, "\x1b[2K", 4);

    int msglen = strlen(editor.msg);
    if (msglen > editor.wincols) {
        msglen = editor.wincols;
    }

    if (msglen && time(NULL) - editor.msg_time < 5) {
        strb_append(sbptr, editor.msg, msglen);
    }
}

void editor_refresh_screen() {
    editor_scroll();

    StringBuffer strb = STRBUF_INIT;
    strb_append(&strb, "\x1b[48;2;17;17;17m", 16);
    strb_append(&strb, "\x1b[?25l\x1b[H", 9);
    editor_draw_rows(&strb);
    editor_status_bar(&strb);
    editor_message_bar(&strb);

    int curpos_size_required = snprintf(NULL, 0, "\x1b[%d;%dH", editor.cy - editor.rowoff + 1, editor.rx - editor.coloff + 1) + 1;

    /* TODO?: It is ideal for the corpus to be allocated dynamically. */
        
        char curpos[curpos_size_required];
    
    snprintf(curpos, sizeof(curpos), "\x1b[%d;%dH", editor.cy - editor.rowoff + 1, editor.rx - editor.coloff + 1);

    strb_append(&strb, curpos, strlen(curpos));

    if (show_editor_cursor)
        strb_append(&strb, "\x1b[?25h", 6);

    write(STDOUT_FILENO, strb.str, strb.len);
    strb_free(&strb);
}

void editor_move_cursor(int key)
{
    size_t rowlen = (editor.cy < editor.numrows) ? editor.rows[editor.cy].len : 0;

    switch (key)
    {
    case ARROW_UP:
        if (editor.cy > 0)
            --editor.cy;
        break;
    case ARROW_DOWN:
        if (editor.cy < editor.numrows - 1)
            ++editor.cy;
        break;
    case ARROW_RIGHT:
        if (editor.cx < rowlen)
            ++editor.cx;
        else if (editor.cy < editor.numrows - 1)
        {
            ++editor.cy;
            editor.cx = 0;
        }
        break;
    case ARROW_LEFT:
        if (editor.cx > 0) {
            --editor.cx;
        } else if (editor.cy > 0) {
            editor.cx = editor.rows[--editor.cy].len;
        }

        break;
    }

    rowlen = (editor.cy < editor.numrows) ? editor.rows[editor.cy].len : rowlen;
    if (editor.cx > rowlen)
        editor.cx = rowlen;
}

/***  ROW OPERATIONS ***/

void editor_insert_row(int at, char *line, size_t len)
{
    EditorRow *new_rows = realloc(editor.rows, (editor.numrows + 1) * sizeof(EditorRow));
    if (new_rows == NULL)
        editor_die("editor_insert_row");

    editor.rows = new_rows;
    // fprintf(debug, "%p\n", e.rows);

    memmove(&editor.rows[at + 1], &editor.rows[at], (editor.numrows - at) * sizeof(EditorRow));
    editor.rows[at].render = NULL;
    editor.rows[at].hl = NULL;
    editor.rows[at].len = len;
    editor.rows[at].text = malloc(len + 1);
    editor.rows[at].is_comment_open = false;
    editor.rows[at].hlen = 0;
    if (editor.rows[at].text == NULL)
        editor_die("editor_insert_row");

    for (int i = at; i <= editor.numrows; ++i)
        editor.rows[i].index = i;

    memcpy(editor.rows[at].text, line, len);
    editor.rows[at].text[len] = '\0';
    ++editor.numrows;

    if (row_update(&editor.rows[at]) == -1)
        editor_die("row_update");
}

void editor_delete_row(int at)
{
    if (at < 0 || at >= editor.numrows)
        return;

    free_row(&editor.rows[at]);
    memmove(&editor.rows[at], &editor.rows[at + 1], (editor.numrows - at - 1) * sizeof(EditorRow));
    for (int i = at; i < editor.numrows; ++i)
        editor.rows[i].index = i;

    --editor.numrows;
}

void editor_insert_newline()
{
    if (editor.cx == 0)
        editor_insert_row(editor.cy, "", 0);
    else
    {
        EditorRow *row = &editor.rows[editor.cy];
        editor_insert_row(editor.cy + 1, &row->text[editor.cx], row->len - editor.cx);
        row = &editor.rows[editor.cy];
        row->len = editor.cx;
        row->text[editor.cx] = '\0';
        row_update(row);
    }

    editor.dirty = true;
    ++editor.cy;
    editor.cx = 0;
}

void editor_insert_char(char c)
{
    if (editor.numrows == 0)
        editor_insert_row(0, "", 0);

    row_insert_char(&editor.rows[editor.cy], editor.cx++, c);
    editor.dirty = true;
}

void editor_delete_char() {
    if (editor.cx == 0 && editor.cy == 0) {
        return;
    }

    EditorRow *row = &editor.rows[editor.cy];
    if (editor.cx == 0) {
        editor.cx = editor.rows[editor.cy - 1].len;
        row_append_string(&editor.rows[editor.cy - 1], row->text, row->len);
        editor_delete_row(editor.cy--);
    } else {
        row_delete_char(row, --editor.cx);
    }

    editor.dirty = true;
}

char *rows_to_str(size_t *buflen)
{
    size_t total_len = 0;
    for (int i = 0; i < editor.numrows; ++i)
        total_len += editor.rows[i].len + 1;
    *buflen = total_len;

    char *str = malloc(total_len);
    if (str == NULL)
        editor_die("rows_to_str");

    char *ptr = str;
    for (int i = 0; i < editor.numrows; ++i)
    {
        memcpy(ptr, editor.rows[i].text, editor.rows[i].len);
        ptr += editor.rows[i].len;
        *ptr = '\n';
        ++ptr;
    }

    return str;
}

/***  USER QUERY PROMPT  ***/

// prompt expects a '%s' format specifier to print what user types
char *editor_prompt(char *prompt, void (*callback)(char *, int))
{
    size_t input_size = 128;
    char *input = malloc(input_size);
    if (input == NULL)
        editor_die("editor_prompt");

    show_editor_cursor = false;

    size_t inputlen = 0;
    input[0] = '\0';

    while (1)
    {
        editor_set_message(prompt, input);

        StringBuffer strb = STRBUF_INIT;
        strb_append(&strb, "\x1b[?25h", 6);
        editor_message_bar(&strb);
        write(STDOUT_FILENO, strb.str, strb.len);
        strb_free(&strb);

        int c = term_read_key();
        if (c == ENTER && inputlen > 0)
        {
            if (callback != NULL)
            {
                callback(input, c);
                continue;
            }

            break;
        }
        else if (c == BACKSPACE || c == CTRL_KEY('h') || c == DELETE)
        {
            if (inputlen > 0)
                input[--inputlen] = '\0';
        }
        else if (c == ESC)
        {
            if (callback != NULL)
                callback(input, c);

            free(input);
            input = NULL;
            break;
        }
        else if (c < 128 && !iscntrl(c))
        {
            if (inputlen == input_size - 1)
            {
                input_size += 128;
                char *newbuf = realloc(input, input_size);
                if (newbuf == NULL)
                {
                    free(input);
                    editor_die("editor_prompt");
                }
            }

            input[inputlen++] = c;
            input[inputlen] = '\0';
        }

        if (callback != NULL)
            callback(input, c);
    }

    editor_set_message("");
    show_editor_cursor = true;
    return input;
}

/***  FIND  ***/

void editor_find_callback(char *query, int key)
{
    static int direction = 1;
    static int last_match = -1;

    if (key == ESC || key == ENTER)
    {
        direction = 1;
        last_match = -1;
        if (key == ESC)
        {
            highlight_match_reset();
            return;
        }
    }

    if (key == ARROW_UP)
        direction = -1;
    else if (key == ARROW_DOWN)
        direction = 1;
    else if (key != ENTER)
        return;

    highlight_match_reset();

    int current = last_match;
    for (int i = 0; i < editor.numrows; ++i)
    {
        current += direction;
        if (current < 0)
            current = editor.numrows - 1;
        else if (current == editor.numrows)
            current = 0;

        char *text = editor.rows[current].text;
        char *match = strstr(text, query);
        if (match != NULL)
        {
            last_match = current;
            editor.cy = current;
            editor.cx = match - text;

            highlight_match_set(&editor.rows[current], row_cx_to_rx(&editor.rows[current], editor.cx), strlen(query));
            editor_refresh_screen();
            return;
        }
    }

    editor_set_message("No results found. Press any key to continue.");
    editor_refresh_screen();
    term_read_key();
}

void editor_find()
{
    char *query = editor_prompt("Find: %s", editor_find_callback);
    if (query != NULL)
        free(query);
}

/***  GO TO LINE  ***/

void editor_goto_line()
{
    char *numstr = editor_prompt("Goto line: %s", NULL);
    if (numstr == NULL)
        return;

    int y = atoi(numstr);
    free(numstr);
    if (y < 1 || y > editor.numrows)
        return;

    editor.cx = 0;

    // if (y - 1 > e.rowoff)
    //     e.rowoff = y;

    editor.cy = y - 1;
}

/***  FILE I/O  ***/

void editor_open(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        editor_die("fopen");

    editor.filename = strdup(filename);
    if (select_syntax_hl(filename) == -1)
        editor_die("selct_syntax_hl");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t len;
    while ((len = getline(&line, &linecap, fp)) != -1)
    {
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            --len;

        editor_insert_row(editor.numrows, line, len);
    }

    free(line);
    fclose(fp);
}

void editor_save_file(void) {
    if (!editor.dirty) {
        return;
    }

    if (editor.filename == NULL) {
        if ((editor.filename = editor_prompt("Save as: %s", NULL)) == NULL) {
            editor_set_message("Save aborted!");
            return;
        }

        if (select_syntax_hl(editor.filename) == -1) {
            editor_die("selct_syntax_hl");
        }
    }

    size_t len;
    char *str = rows_to_str(&len);

    int fd = open(editor.filename, O_RDWR | O_CREAT, 0644);

    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if ((size_t)write(fd, str, len) == len) {
                close(fd);
                free(str);
                editor_set_message("File is successfully saved.", len);
                editor.dirty = false;
                return;
            }
        }
        close(fd);
    }

    free(str);
    editor_set_message("Can't save file! %s", strerror(errno));
}

/***  EDITOR INPUT  ***/

void editor_process_key(void) {
    static int quit_times = DIRTY_QUIT_TIMES;

    int key = term_read_key();
    switch (key) {
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_RIGHT:
        case ARROW_LEFT:
            editor_move_cursor(key);
            break;

        case HOME:
            editor.cx = 0;
            break;

        case END:
            if (editor.cy < editor.numrows) {
                editor.cx = editor.rows[editor.cy].len;
            }

            break;

        case PAGE_UP:
        case PAGE_DOWN:
            if (editor.numrows == 0) {
                return;
            }

            if (key == PAGE_UP) {
                editor.cy = editor.rowoff;
            } else {
                editor.cy = editor.rowoff + editor.winrows - 1;
                if (editor.cy > editor.numrows) {
                    editor.cy = editor.numrows - 1;
                }
            }

            for (unsigned short times = editor.winrows; times > 0; --times) {
                editor_move_cursor(key == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }

            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DELETE:
            if (key == DELETE) {
                editor_move_cursor(ARROW_RIGHT);
            }

            editor_delete_char();
            break;

        case ENTER:
            editor_insert_newline();
            break;

        case CTRL_KEY('q'):
            --quit_times;
            if (editor.dirty && quit_times > 0) {
                editor_set_message(
                    "WARNING! File has unsaved changes. Press Ctrl+Q %d more time%s to quit.",
                    quit_times,
                    quit_times == 1 ? ""  : "s"
                );
                return;
            }

            term_exit_alt_screen();
            term_disable_raw_mode();
            exit(0);
            break;

        case CTRL_KEY('s'):
            editor_save_file();
            break;

        case CTRL_KEY('f'):
            editor_find();
            break;

        case CTRL_KEY('g'):
            editor_goto_line();
            break;

        case CTRL_KEY('l'):
        case ESC:
            break;

        case '(':
        case '[':
        case '{':
            editor_insert_char(key);
            editor_insert_char((key == '(') ? ')' : ((key == '[') ? ']' : '}'));
            --editor.cx;
            break;

        case '\'':
        case '"':
            editor_insert_char(key);
            editor_insert_char(key);
            --editor.cx;
            break;

        default:
            editor_insert_char(key);
    }

    quit_times = DIRTY_QUIT_TIMES;
}
