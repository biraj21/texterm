# Escape Sequences

Escape Sequences are sequences of bytes that start with escape character (`\033` or `\x1b` or `\e`), mostly followed by a `[`. They represent some special keys such as arrow keys, function keys, etc. They also act as functions to instruct the terminal to do various tasks, such as coloring text, moving cursor around, clearing parts of screen, etc. You can read more about Escape Sequences on [Wikipedia](https://en.wikipedia.org/wiki/ANSI_escape_code).

## Escape Sequences For Keys

### Arrow Keys

<pre>
<b>Arrow UP</b>    = ESC + [ + A
<b>Arrow DOWN</b>  = ESC + [ + B
<b>Arrow RIGHT</b> = ESC + [ + C
<b>Arrow LEFT</b>  = ESC + [ + D
</pre>

### Function Keys

<pre>
<b>f1</b>  = ESC + O + P
<b>f2</b>  = ESC + O + Q
<b>f3</b>  = ESC + O + R
<b>f4</b>  = ESC + O + S
<b>f5</b>  = ESC + [ + 1 + 5 + ~
<b>f6</b>  = ESC + [ + 1 + 7 + ~
<b>f7</b>  = ESC + [ + 1 + 8 + ~
<b>f8</b>  = ESC + [ + 1 + 9 + ~
<b>f9</b>  = ESC + [ + 2 + 0 + ; + 5 + ~
<b>f12</b> = ESC + [ + 2 + 4 + ~
</pre>

### Other Keys

<pre>
<b>Home</b>      = ESC + [ + H
          = ESC + [ + 1 + ~
          = ESC + [ + 7 + ~
<b>End</b>       = ESC + [ + F
          = ESC + [ + 4 + ~
          = ESC + [ + 8 + ~
<b>Insert</b>    = ESC + [ + 2 + ~
<b>Delete</b>    = ESC + [ + 3 + ~
<b>Page Up</b>   = ESC + [ + 5 + ~
<b>Page Down</b> = ESC + [ + 6 + ~

<b>CTRL + SHIFT + ARROW RIGHT</b> = ESC + [ + 1 + ; + 6 + C
<b>CTRL + SHIFT + ARROW LEFT</b>  = ESC + [ + 1 + ; + 6 + D

<b>ESC + SHIFT + ARROW UP</b>     = ESC + [ + 1 + ; + 2 + A
<b>ESC + SHIFT + ARROW DOWN</b>   = ESC + [ + 1 + ; + 2 + B
<b>ESC + SHIFT + ARROW RIGHT</b>  = ESC + [ + 1 + ; + 2 + C
<b>ESC + SHIFT + ARROW LEFT</b>   = ESC + [ + 1 + ; + 2 + D
</pre>

## Escape Sequences As Functions

### Syntax: `\e[<args>f`

- `<args>` - semicolon separeted function arguments
- `f` - function name

## List of some functions

- `\e[nA` - Move cursor up by <i>n</i> positions
- `\e[nB` - Move cursor down by <i>n</i> positions
- `\e[nC` - Move cursor right by <i>n</i> positions
- `\e[nD` - Move cursor lef by <i>n</i> positions
- `\e[nJ` - Erase in display
  - 0 - From cursor positon to end of screen
  - 1 - From start of screen to cursor postion
  - 2 - All display
- `\e[nK` - Erase in line
  - 0 - From cursor positon to end of line
  - 1 - From start of line to cursor position
  - 2 - Whole line
- `\e[r;cH` - Move cursor to <i>r<sup>th</sup></i> row & <i>c<sup>th</sup></i> column, starts with 1 not 0
- `\e[6n` - Get cursor position (responds with `\e[r;cR`)
- `\e[?1049h` - Enter alternate screen saving cursor positon
- `\e[?1049l` - Exit alternate screen restoring cursor position
- `\e[?25l` - Hide cursor
- `\e[?25h` - Show cursor
