#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int get_cursor_position(int *row, int *col)
{
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    char res[16]; // stores response form CPR
    int i;
    for (i = 0; i < sizeof(res); ++i)
    {
        if (read(STDIN_FILENO, &res[i], 1) != 1 || res[i] == 'R')
            break;
    }
    res[i] = '\0';

    if (res[0] != '\x1b' || res[1] != '[')
        return -1;

    if (sscanf(&res[2], "%d;%d", row, col) != 2)
        return -1;

    return 0;
}

int get_win_size(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return get_cursor_position(rows, cols);
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}