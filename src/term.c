#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "editor.h"
#include "term.h"

extern Editor editor;

struct termios orignal_term_state; // original settings of the terminal

bool entered_alt_screen = false;
bool raw_mode_enabled = false;

void term_enter_alt_screen(void) {
    if (write(STDOUT_FILENO, "\x1b[?1049h\x1b[H", 11) != 11) {
        editor_die("failed to enter alternate screen");
    }

    entered_alt_screen = true;
}

void term_exit_alt_screen(void) {
    if (write(STDOUT_FILENO, "\x1b[?1049l", 8) != 8) {
        editor_die("falied to exit alternate screen");
    }
}

void term_enable_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &orignal_term_state) == -1) {
        editor_die("falied to enable raw mode (tcgetattr)");
    }

    struct termios term = orignal_term_state;

    // from man pages (man cfmakeraw -> Raw Mode -> cfmakeraw())
    // read /notes/raw_mode.md
    term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    term.c_oflag &= ~OPOST;
    term.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    term.c_cflag &= ~(CSIZE | PARENB);
    term.c_cflag |= CS8;

    // to read within 100ms
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1; // 1 * 1/10th seconds = 100ms timout

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1) {
        editor_die("falied to enable raw mode (tcsetattr)");
    }

    raw_mode_enabled = true;
}

void term_disable_raw_mode(void) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orignal_term_state) == -1) {
        editor_die("falied to disable raw mode (tcsetattr)");
    }
}

int term_read_key(void) {
    ssize_t bytes_read;
    char c;
    do {
        bytes_read = read(STDIN_FILENO, &c, 1);
        // in Cygwin, when read() times out it returns -1 and sets errno
        // to EAGAIN, instead of just returning 0
        if (bytes_read == -1 && errno != EAGAIN) {
            editor_die("falied to read input");
        }
    } while (bytes_read != 1);

    if (c != ESC) {
        return c;
    }

    // for reading escape sequences to check if the user pressed arrow keys,
    // function keys, HOME, END, etc.
    char seq[5];

    read_esc_seq:
    // 2nd byte
    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
        return ESC;
    }

    /*
     * It is possible to get a combination like ESC + Arrow Up, in which case
     * the sequence will be ESC, ESC, [, A. In such cases, I want to ignore the
     * first ESC and just return Arrow Up.
     */

    if (seq[0] == ESC) {
        goto read_esc_seq;
    }

    if (seq[0] == '[') {
        // 3rd byte
        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            return ESC;
        }

        if (seq[1] >= '0' && seq[1] <= '9') {
            // 4th byte
            if (read(STDIN_FILENO, &seq[2], 1) != 1) {
                return ESC;
            }

            if (seq[2] != '~') {
                // 5th byte
                if (read(STDIN_FILENO, &seq[3], 1) == 1) {
                    read(STDIN_FILENO, &seq[4], 1); // 6th byte
                }

                return ESC;
            }

            switch (seq[1]) {
                case '1':
                case '7':
                    return HOME;
                case '3':
                    return DELETE;
                case '4':
                case '8':
                    return END;
                case '5':
                    return PAGE_UP;
                case '6':
                    return PAGE_DOWN;
            }
        } else {
            switch (seq[1]) {
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
    } else if (seq[0] == 'O') {
        // 3rd byte
        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            return ESC;
        }

        switch (seq[1]) {
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

    return ESC;
}