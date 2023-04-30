#ifndef TERM_H
#define TERM_H

enum Keys {
    ENTER = 13, // '\r',
    ESC = 27, // '\x1b',
    BACKSPACE = 127,
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_RIGHT,
    ARROW_LEFT,
    DELETE,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
};

/*
 * CTRL_KEY macro is used to check if the key pressed is Ctrl+<alphabet>
 *
 * Ctrl key combined with the alphabetic keys maps to 1–26.
 *
 * ASCII of 'b' is 98, which is 01100010 in binary.
 * 0x1f is 00011111 in binary.
 * 00011111 & 01100010 = 00000010 = 2, which is Ctrl+b.
 *
 * ASCII of 'B' is 66, which is 01000010 in binary.
 * 00011111 & 01000010 = 00000010 = 2,  which is Ctrl+B.
 *
 * So it works regardless of the case.
 *
 * This mirrors what the Ctrl key does in the terminal: it strips bits 5 and 6
 * from whatever key you press in combination with Ctrl, and sends that.
 * (By convention, bit numbering starts from 0.) The ASCII character set seems
 * to be designed this way on purpose. (It is also similarly designed so that
 * you can set and clear bit 5 to switch between lowercase and uppercase.)
 */
#define CTRL_KEY(k) (k & 0x1f)

extern bool entered_alt_screen;
extern bool raw_mode_enabled;

void term_enter_alt_screen(void);
void term_exit_alt_screen(void);
void term_enable_raw_mode(void);
void term_disable_raw_mode(void);
int term_read_key(void);

#endif