#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <termios.h>
#include <time.h>
#include "./highlight.h"
#include "./rows.h"

#define NAME "Texterm"
#define VERSION "0.0.1"
#define DIRTY_QUIT_TIMES 3
#define CTRL_KEY(k) (k & 0x1f)

typedef struct
{
    int cx, cy;
    int rx;
    int rowoff, coloff;
    int winrows, wincols;
    int numrows;
    EditorRow *rows;
    char *filename;
    char msg[80];
    time_t msg_time;
    bool dirty;
    struct termios orig_term; // original state of the terminal
    int tab_stop;
    SyntaxInfo *syntax;
} Editor;

enum EditorKeys
{
    ENTER = '\r',
    ESC = '\x1b',
    BACKSPACE = 127,
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_RIGHT,
    ARROW_LEFT,
    DEL,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN
};

#endif