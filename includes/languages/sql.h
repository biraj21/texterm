#ifndef SQL_H
#define SQL_H

#include <stddef.h>

char *sql_exts[] = {".sql", NULL};

char *sql_commands[] = {
    "SELECT", "UPDATE", "CREATE",
    "DROP", "DELETE", "INSERT", "INTO", "ALTER",
    "DATABASE", "TABLE",
    "GRANT", "REVOKE",
    "COMMIT", "ROLLBACK",
    "INDEX",
    "BACKUP", "TO", "DISK", "WITH", "DIFFERENTIAL", "BETWEEN",

    "FORM", "WHERE", "LIKE", "IN", "AS", "HAVING",
    "UNION", "JOIN", "GROUP", "LEFT", "RIGHT", "INNER",
    "NOT", "NULL", "PRIMARY", "KEY",
    "UNIQUE", "DEFAULT", "AUTO_INCREMENT", "TIMESTAMP",

    // types
    "VARCHAR|", "INT|", "TINYINT|", "BIGINT|", "FLOAT|", "REAL|",
    "DATE|", "TIME|", "DATETIME|", "NCHAR|", "NVARCHAR|", "NTEXT|",
    "BINARY|", "VARBINARY|",
    "CLOB|", "BLOB|", "XML|", "CURSOR|", "TABLE|", NULL};

char *sql_functions[] = {
    // aggregrate functions
    "AVG", "COUNT", "FIRST", "LAST", "MAX", "MIN", "SUM",

    // scalar functions
    "UCASE", "LCASE", "MID", "LEN", "ROUND", "NOW", "FORMAT", NULL};

#endif