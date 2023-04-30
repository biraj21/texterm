# Raw Mode
The terminal, by default, is in canonical or cooking mode, which means that it will process input before sending it to the program. For example, buffering input until newline or EOF is encountered. So we want enter raw mode where these things are turned off. And we'll have to disable several flags to do that.

## local flags (``c_lflag``)
* ECHO - prints the character that was just typed
* ICANON - canonical mode processes input before sending it
* IEXTEN - also processes input in some way (idk)
* ISIG - interrupt signals (ctrl+c and ctrl+z)

## input flags (``c_iflag``)
* BRKINT - send a SIGINT (ctrl+c) when a break condition is detected
* ICRNL - '\r' (13) is translated to '\n' (10)
* INPCK - enables input parity checking
* ISTRIP - strips the 8th bit off characters
* IXON - ctrl+s to pause output transmission, ctrl+q to resume it

## output flags (``c_oflag``)
* OPOST - post-processes output. For eg:
    * ONLCR - '\n' is translated to "\r\n"
    * OCRNL - '\r\ is translated to '\n'
    * OLCUC - lowercase chars are translated to uppercase

## control flags (``c_cflag``)
* CS8 - a mask that sets the character size to 8 bits

Read more about it in Linux manual with
```
man termios
```