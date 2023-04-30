# Contributing Guidlines

- Follow the [project structure](#project-structure).
- Follow the [code guidlines](#code-guidlines).

## Project Structure

```
texterm/
  - src/
    - syntax/
      - <lang>.h
    - main.c
    - <file>.h
    - <file>.c
    - ...
```

## Code Guidlines

- Use spaces instead of tabs.
- Keep indent size 4.
- Follow the [naming conventions](#naming-conventions).

### Naming Conventions

- **variable names** - snake_case
  ```c
  int last_match = -1;
  ```
- **function names** - snake_case
  Example:

  ```c
  void term_enter_alt_screen(void) {
      ...
  }
  ```

- **structs / enums** - _typedefed_ & CamelCase

  Example:

  ```c
  typedef struct {
      ...
  } Editor;
  ```
