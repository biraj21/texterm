#ifndef C_H
#define C_H

#include <stddef.h>

char *c_exts[] = {".c", ".h", ".cpp", NULL};

char *c_keywords[] = {
    "auto", "break", "case", "continue", "do", "default", "else",
    "extern", "for", "if", "goto", "register", "return", "sizeof",
    "switch", "volatile", "while", "restrict",

    /* C11 */
    "_Alignas",
    "_Alignof",
    "_Atomic|",
    "_Generic",
    "_Noreturn",
    "_Static_assert",
    "_Thread_local|",

    /* C99 */
    "_Bool|",
    "_Complex|",
    "_Imaginary|",

    /* C++ */
    "and",
    "and_eq",
    "asm",
    "bitand",
    "bitor",
    "catch",
    "class",
    "compl",
    "const_cast",
    "delete",
    "dynamic_cast",
    "explicit",
    "export",
    "friend",
    "mutable",
    "namespace",
    "new",
    "not",
    "not_eq",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "reinterpret_cast",
    "static_cast",
    "template",
    "this",
    "throw",
    "try",
    "typeid",
    "typename",
    "virtual",
    "wchar_t|",
    "xor",
    "xor_eq",

    /* C++11 */
    "alignas",
    "alignof",
    "constexpr",
    "decltype",
    "final",
    "noexcept",
    "nullptr",
    "override",
    "static_assert",
    "thread_local",

    /* C++20 */
    "char8_t|",
    "char16_t|",
    "char32_t|",
    "concept",
    "consteval",
    "constinit",
    "co_await",
    "co_return",
    "co_yield",
    "import",
    "module",
    "requires",

    // types, variables & functions related
    "char|", "const|", "double|", "enum|", "float| ", "int|", "long|",
    "short|", "signed|", "static|", "struct|", "typedef|", "union|", "unsigned|",
    "void|", NULL};

char *c_preprocs[] = {
    "define", "defined", "elif", "else", "endif", "error", "if", "ifdef",
    "ifndef", "include", "undef", "pragma",

    /* C++17 */
    "__has_include",

    /* C++20 */
    "__has_cpp_attribute",

    NULL};

#endif
