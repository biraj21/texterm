# Escape Sequences

Escape sequences always start with an escape character (``\e`` or ``\x1b``), followed by a ``[``. They instruct the terminal to do various tasks, such as coloring text, moving cursor around, clearing parts of screen, etc.

Syntax: ``\e[<args>f]``
* ``<args>`` : semicolon separeted function arguments
* ``f``: function name

<br>

## List of some functions
* ``\e[aA`` - Move cursor up <i>a</i> positions <b>up</b>
* ``\e[aB`` - Move cursor up <i>a</i> positions <b>down</b>
* ``\e[aC`` - Move cursor up <i>a</i> positions <b>right</b>
* ``\e[aD`` - Move cursor up <i>a</i> positions <b>left</b>
* ``\e[aJ`` - Erase in display
    * 0 - From cursor positon to end of screen 
    * 1 - From start of screen to cursor postion
    * 2 - All display
* ``\e[aK`` - Erase in line
    * 0 - From cursor positon to end of line 
    * 1 - From start of screen to cursor line
    * 2 - Whole line
* ``\e[r;cH`` - Move cursor to <i>r<sup>th</sup></i> row & <i>c<sup>th</sup></i> column
* ``\e[6n`` - Get cursor position (responds with ``\e[r;cR``)

