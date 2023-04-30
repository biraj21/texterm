#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stddef.h>
#include <termios.h>
#include <time.h>
#include "highlight.h"
#include "rows.h"

#define NAME "Texterm"
#define VERSION "0.3.0"
#define DIRTY_QUIT_TIMES 3

typedef struct {
    unsigned short cx, cy; // cursor position
    unsigned short rx;
    unsigned short rowoff, coloff;
    unsigned short winrows, wincols;
    unsigned short numrows;
    EditorRow *rows;
    char *filename;
    char msg[80];
    time_t msg_time;
    bool dirty;
    int tab_stop;
    SyntaxInfo *syntax;
} Editor;

void editor_die(char *str);
void editor_init(void);
void editor_set_message(char *msg, ...);
void editor_refresh_screen(void);
void editor_process_key(void);
void editor_open(const char *filename);

#endif