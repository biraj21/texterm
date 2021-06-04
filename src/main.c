#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "../includes/editor.h"
#include "../includes/highlight.h"
#include "../includes/rows.h"
#include "../includes/strbuf.h"
#include "../includes/winsize.h"

#define tc_enter_alt_screen() write(STDOUT_FILENO, "\x1b[?1049h\x1b[H", 11);
#define tc_exit_alt_screen() write(STDOUT_FILENO, "\x1b[?1049l", 8);

void die(char *str);
void enable_raw_mode();
void on_win_resize();
void editor_init();
void editor_set_message(char *msg, ...);
void editor_refresh_screen();
void editor_process_key();
void editor_open(const char *filename);
void free_memory();

Editor e;
bool show_editor_cursor = true;
// FILE *debug = NULL;

int main(int argc, const char *argv[])
{
    atexit(free_memory);
    signal(SIGWINCH, on_win_resize);

    enable_raw_mode();
    editor_init();
    tc_enter_alt_screen();

    // debug = fopen("/dev/pts/1", "w");
    // if (debug == NULL)
    //     die("fopen");

    if (argc > 1)
        editor_open(argv[1]);

    char *help[] = {
        "Ctrl+F = Find (Enter - search | ESC - cancel | Arrow Up/Down - move)",
        "Ctrl+G = Goto line",
        "Ctrl+Q = Quit | Ctrl+S = Save",
    };

    editor_set_message(help[time(NULL) % 3]);

    while (1)
    {
        editor_refresh_screen();
        editor_process_key();
    }

    // fclose(debug);
    return 0;
}

void die(char *str)
{
    tc_exit_alt_screen();
    perror(str);
    exit(1);
}

/***  TERMINAL  ***/

void disable_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &e.orig_term) == -1)
        die("tcsetattr");
}

void enable_raw_mode()
{
    if (tcgetattr(STDIN_FILENO, &e.orig_term) == -1)
        die("tcgetattr");

    struct termios term_state = e.orig_term;

    term_state.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    term_state.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    term_state.c_cflag &= ~OPOST;
    term_state.c_cflag |= CS8;

    term_state.c_cc[VMIN] = 0;
    term_state.c_cc[VTIME] = 1;

    atexit(disable_raw_mode);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_state) == -1)
        die("tcsetattr");
}

int read_key()
{
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }

    if (c == ESC)
    {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return ESC;

        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return ESC;

        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1 || seq[2] != '~')
                    return ESC;

                switch (seq[1])
                {
                case '3':
                    return DEL;
                case '1':
                case '7':
                    return HOME;
                case '4':
                case '8':
                    return END;
                case '5':
                    return PAGE_UP;
                case '6':
                    return PAGE_DOWN;
                }
            }
            else
            {
                switch (seq[1])
                {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                case 'F':
                    return END;
                case 'H':
                    return HOME;
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch (seq[1])
            {
            case 'F':
                return END;
            case 'H':
                return HOME;
            }
        }

        return ESC;
    }

    return c;
}

/***  EDITOR  ***/

void on_win_resize()
{
    if (get_win_size(&e.winrows, &e.wincols) == -1)
        die("get_win_size");

    e.winrows -= 2;
    editor_refresh_screen();
}

void editor_init()
{
    e.cx = e.cy = 0;
    e.rx = 0;
    e.rowoff = e.coloff = 0;
    e.numrows = 0;
    e.rows = NULL;
    e.filename = NULL;
    e.dirty = false;
    e.msg[0] = '\0';
    e.msg_time = 0;
    e.tab_stop = 4;
    e.syntax = NULL;

    if (get_win_size(&e.winrows, &e.wincols) == -1)
        die("get_win_size");

    // last 2nd row is status bar
    // last row is for prompt & message
    e.winrows -= 2;
}

void editor_scroll()
{
    e.rx = 0;
    if (e.cy < e.numrows)
        e.rx = row_cx_to_rx(&e.rows[e.cy], e.cx);

    if (e.cy < e.rowoff)
        e.rowoff = e.cy;
    else if (e.cy >= e.rowoff + e.winrows)
        e.rowoff = e.cy + 1 - e.winrows;

    if (e.rx < e.coloff)
        e.coloff = e.rx;
    else if (e.rx >= e.coloff + e.wincols)
        e.coloff = e.rx + 1 - e.wincols;
}

void editor_draw_rows(StringBuffer *sbptr)
{
    for (int y = 0; y < e.winrows; ++y)
    {
        int iy = y + e.rowoff, textlen = 0;
        if (iy < e.numrows)
        {
            textlen = e.rows[iy].rlen - e.coloff;
            if (textlen > e.wincols)
                textlen = e.wincols;

            if (textlen > 0)
            {
                highlight(&e.rows[iy], e.coloff, textlen, sbptr);
                if (e.rows[iy].render[e.coloff + textlen - 1] != HL_DEFAULT)
                    strb_append(sbptr, "\x1b[48;2;17;17;17m\x1b[39m", 21);
            }
        }
        else if (e.numrows == 0 && y == e.winrows / 2 - 1)
        {
            strb_append(sbptr, "~", 1);

            char ver_info[] = "Version " VERSION;
            int verlen = sizeof(ver_info) - 1;

            if (e.wincols < verlen + 10)
                continue;

            int namelen = sizeof(NAME) - 1;
            int padlen = (e.wincols - namelen) / 2;

            for (int i = 1; i < padlen; ++i)
                strb_append(sbptr, " ", 1);

            strb_append(sbptr, NAME, namelen);
            strb_append(sbptr, "\x1b[K\r\n~", 6);

            ++y;

            padlen = (e.wincols - verlen) / 2;
            for (int i = 1; i < padlen; ++i)
                strb_append(sbptr, " ", 1);

            strb_append(sbptr, ver_info, verlen);
        }
        else
            strb_append(sbptr, "~", 1);

        strb_append(sbptr, "\x1b[K", 3);
        strb_append(sbptr, "\r\n", 2);
    }
}

void editor_status_bar(StringBuffer *sbptr)
{
    strb_append(sbptr, "\x1b[48;2;221;221;221m\x1b[38;2;0;0;0m", 32);

    int namelen = 10;
    if (e.filename == NULL)
        strb_append(sbptr, "[new file]", namelen);
    else
    {
        namelen = strlen(e.filename);
        strb_append(sbptr, e.filename, namelen);
    }

    if (e.dirty)
    {
        strb_append(sbptr, "*", 1);
        ++namelen;
    }

    char cur_info[20];
    snprintf(cur_info, sizeof(cur_info), "Line %d, Col %d", e.cy + 1, e.rx + 1);

    int curlen = strlen(cur_info);
    for (int i = namelen; i < e.wincols - curlen; ++i)
        strb_append(sbptr, " ", 1);

    strb_append(sbptr, cur_info, curlen);
    strb_append(sbptr, "\x1b[m\r\n", 5);
}

void editor_set_message(char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vsnprintf(e.msg, sizeof(e.msg), msg, args);
    va_end(args);
    e.msg_time = time(NULL);
}

void editor_message_bar(StringBuffer *sbptr)
{
    char pos[16];
    snprintf(pos, sizeof(pos), "\x1b[%d;1H", e.winrows + 2);

    strb_append(sbptr, pos, strlen(pos));
    strb_append(sbptr, "\x1b[2K", 4);

    int msglen = strlen(e.msg);
    if (msglen > e.wincols)
        msglen = e.wincols;

    if (msglen && time(NULL) - e.msg_time < 5)
        strb_append(sbptr, e.msg, msglen);
}

void editor_refresh_screen()
{
    editor_scroll();

    StringBuffer strb = STRBUF_INIT;
    strb_append(&strb, "\x1b[48;2;17;17;17m", 16);
    strb_append(&strb, "\x1b[?25l\x1b[H", 9);
    editor_draw_rows(&strb);
    editor_status_bar(&strb);
    editor_message_bar(&strb);

    char curpos[16];
    snprintf(curpos, sizeof(curpos), "\x1b[%d;%dH", e.cy - e.rowoff + 1, e.rx - e.coloff + 1);

    strb_append(&strb, curpos, strlen(curpos));

    if (show_editor_cursor)
        strb_append(&strb, "\x1b[?25h", 6);

    write(STDOUT_FILENO, strb.str, strb.len);
    strb_free(&strb);
}

void editor_move_cursor(int key)
{
    int rowlen = (e.cy < e.numrows) ? e.rows[e.cy].len : 0;

    switch (key)
    {
    case ARROW_UP:
        if (e.cy > 0)
            --e.cy;
        break;
    case ARROW_DOWN:
        if (e.cy < e.numrows - 1)
            ++e.cy;
        break;
    case ARROW_RIGHT:
        if (e.cx < rowlen)
            ++e.cx;
        else if (e.cy < e.numrows - 1)
        {
            ++e.cy;
            e.cx = 0;
        }
        break;
    case ARROW_LEFT:
        if (e.cx > 0)
            --e.cx;
        else if (e.cy > 0)
            e.cx = e.rows[--e.cy].len;
        break;
    }

    rowlen = (e.cy < e.numrows) ? e.rows[e.cy].len : rowlen;
    if (e.cx > rowlen)
        e.cx = rowlen;
}

/***  ROW OPERATIONS ***/

void editor_insert_row(int at, char *line, size_t len)
{
    EditorRow *new_rows = realloc(e.rows, (e.numrows + 1) * sizeof(EditorRow));
    if (new_rows == NULL)
        die("editor_insert_row");

    e.rows = new_rows;
    // fprintf(debug, "%p\n", e.rows);

    memmove(&e.rows[at + 1], &e.rows[at], (e.numrows - at) * sizeof(EditorRow));
    e.rows[at].render = NULL;
    e.rows[at].hl = NULL;
    e.rows[at].len = len;
    e.rows[at].text = malloc(len + 1);
    e.rows[at].is_comment_open = false;
    e.rows[at].hlen = 0;
    if (e.rows[at].text == NULL)
        die("editor_insert_row");

    for (int i = at; i <= e.numrows; ++i)
        e.rows[i].index = i;

    memcpy(e.rows[at].text, line, len);
    e.rows[at].text[len] = '\0';
    ++e.numrows;

    if (row_update(&e.rows[at]) == -1)
        die("row_update");
}

void editor_delete_row(int at)
{
    if (at < 0 || at >= e.numrows)
        return;

    free_row(&e.rows[at]);
    memmove(&e.rows[at], &e.rows[at + 1], (e.numrows - at - 1) * sizeof(EditorRow));
    for (int i = at; i < e.numrows; ++i)
        e.rows[i].index = i;

    --e.numrows;
}

void editor_insert_newline()
{
    if (e.cx == 0)
        editor_insert_row(e.cy, "", 0);
    else
    {
        EditorRow *row = &e.rows[e.cy];
        editor_insert_row(e.cy + 1, &row->text[e.cx], row->len - e.cx);
        row = &e.rows[e.cy];
        row->len = e.cx;
        row->text[e.cx] = '\0';
        row_update(row);
    }

    e.dirty = true;
    ++e.cy;
    e.cx = 0;
}

void editor_insert_char(char c)
{
    if (e.numrows == 0)
        editor_insert_row(0, "", 0);

    row_insert_char(&e.rows[e.cy], e.cx++, c);
    e.dirty = true;
}

void editor_delete_char()
{
    if (e.cx == 0 && e.cy == 0)
        return;

    EditorRow *row = &e.rows[e.cy];
    if (e.cx == 0)
    {
        e.cx = e.rows[e.cy - 1].len;
        row_append_string(&e.rows[e.cy - 1], row->text, row->len);
        editor_delete_row(e.cy--);
    }
    else
        row_delete_char(row, --e.cx);

    e.dirty = true;
}

char *rows_to_str(size_t *buflen)
{
    size_t total_len = 0;
    for (int i = 0; i < e.numrows; ++i)
        total_len += e.rows[i].len + 1;
    *buflen = total_len;

    char *str = malloc(total_len);
    if (str == NULL)
        die("rows_to_str");

    char *ptr = str;
    for (int i = 0; i < e.numrows; ++i)
    {
        memcpy(ptr, e.rows[i].text, e.rows[i].len);
        ptr += e.rows[i].len;
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
        die("editor_prompt");

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

        int c = read_key();
        if (c == ENTER && inputlen > 0)
        {
            if (callback != NULL)
            {
                callback(input, c);
                continue;
            }

            break;
        }
        else if (c == BACKSPACE || c == CTRL_KEY('h') || c == DEL)
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
                    die("editor_prompt");
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
    for (int i = 0; i < e.numrows; ++i)
    {
        current += direction;
        if (current < 0)
            current = e.numrows - 1;
        else if (current == e.numrows)
            current = 0;

        char *text = e.rows[current].text;
        char *match = strstr(text, query);
        if (match != NULL)
        {
            last_match = current;
            e.cy = current;
            e.cx = match - text;

            highlight_match_set(&e.rows[current], row_cx_to_rx(&e.rows[current], e.cx), strlen(query));
            editor_refresh_screen();
            return;
        }
    }

    editor_set_message("No results found. Press any key to continue.");
    editor_refresh_screen();
    read_key();
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
    if (y < 1 || y > e.numrows)
        return;

    e.cx = 0;

    // if (y - 1 > e.rowoff)
    //     e.rowoff = y;

    e.cy = y - 1;
}

/***  FILE I/O  ***/

void editor_open(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        die("fopen");

    e.filename = strdup(filename);
    if (select_syntax_hl(filename) == -1)
        die("selct_syntax_hl");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t len;
    while ((len = getline(&line, &linecap, fp)) != -1)
    {
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            --len;

        editor_insert_row(e.numrows, line, len);
    }

    free(line);
    fclose(fp);
}

void editor_save_file()
{
    if (!e.dirty)
        return;

    if (e.filename == NULL)
    {
        if ((e.filename = editor_prompt("Save as: %s", NULL)) == NULL)
        {
            editor_set_message("Save aborted!");
            return;
        }

        if (select_syntax_hl(e.filename) == -1)
            die("selct_syntax_hl");
    }

    size_t len;
    char *str = rows_to_str(&len);

    int fd = open(e.filename, O_RDWR | O_CREAT, 0644);

    if (fd != -1)
    {
        if (ftruncate(fd, len) != -1)
        {
            if (write(fd, str, len) == len)
            {
                close(fd);
                free(str);
                editor_set_message("File is successfully saved.", len);
                e.dirty = false;

                return;
            }
        }
        close(fd);
    }

    free(str);
    editor_set_message("Can't save file! %s", strerror(errno));
}

/***  EDITOR INPUT  ***/

void editor_process_key()
{
    static int quit_times = DIRTY_QUIT_TIMES;

    int key = read_key();
    switch (key)
    {
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_RIGHT:
    case ARROW_LEFT:
        editor_move_cursor(key);
        break;

    case HOME:
        e.cx = 0;
        break;

    case END:
        if (e.cy < e.numrows)
            e.cx = e.rows[e.cy].len;
        break;

    case PAGE_UP:
    case PAGE_DOWN:
        if (e.numrows == 0)
            return;

        if (key == PAGE_UP)
            e.cy = e.rowoff;
        else
        {
            e.cy = e.rowoff + e.winrows - 1;
            if (e.cy > e.numrows)
                e.cy = e.numrows - 1;
        }

        int times = e.winrows;
        while (times--)
            editor_move_cursor(key == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        break;

    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL:
        if (key == DEL)
            editor_move_cursor(ARROW_RIGHT);
        editor_delete_char();
        break;

    case ENTER:
        editor_insert_newline();
        break;

    case CTRL_KEY('q'):
        if (e.dirty && --quit_times > 0)
        {
            editor_set_message(
                "WARNING! File has unsaved changes."
                " Press Ctrl+Q %d more times to quit.",
                quit_times);
            return;
        }

        tc_exit_alt_screen();
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
        --e.cx;
        break;

    case '\'':
    case '"':
        editor_insert_char(key);
        editor_insert_char(key);
        --e.cx;
        break;

    default:
        editor_insert_char(key);
    }

    quit_times = DIRTY_QUIT_TIMES;
}

void free_memory()
{
    for (int y = 0; y < e.numrows; ++y)
        free_row(&e.rows[y]);

    free(e.rows);
    free(e.filename);
}
