#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include "editor.h"
#include "highlight.h"
#include "rows.h"
#include "strbuf.h"
#include "term.h"
#include "winsize.h"

void on_win_resize();
void free_memory(void);

extern Editor editor;

int main(int argc, const char *argv[]) {
    if (argc > 2) {
        editor_die("too many arguments");
    }

    atexit(free_memory);
    signal(SIGWINCH, on_win_resize);

    term_enter_alt_screen();
    term_enable_raw_mode();

    editor_init();

    if (argc > 1) {
        editor_open(argv[1]);
    }

    char *help[] = {
        "Ctrl+F = Find (Enter - search | ESC - cancel | Arrow Up/Down - move)",
        "Ctrl+G = Go to a line",
        "Ctrl+Q = Quit | Ctrl+S = Save",
    };

    srand(time(NULL));
    editor_set_message(help[rand() % 3]);

    while (true) {
        editor_refresh_screen();
        editor_process_key();
    }

    return EXIT_SUCCESS;
}

void on_win_resize() {
    if (get_win_size(&editor.winrows, &editor.wincols) == -1) {
        editor_die("on_win_resize -> get_win_size");
    }

    // subtracting 2 cuz last 2nd row is status bar & last row is for prompt & message
    editor.winrows -= 2;
    editor_refresh_screen();
}

void free_memory(void) {
    for (int y = 0; y < editor.numrows; ++y) {
        free_row(&editor.rows[y]);
    }

    free(editor.rows);
    free(editor.filename);
}