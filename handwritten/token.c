#include <stdio.h>
#include <string.h>

const char * keywords[] = {
    "INT",   "WHILE", "IF",  "ELSE", "RETURN", "READ",
    "WRITE", "BEGIN", "END", "MAIN", "REAL",
};
const int kw_size = sizeof(keywords) / sizeof(keywords[0]);

char * print_token(int token) {
    static char* token_strs[] = {
        "==",
        "!=",
        ":=",
        "<=",
        ">=",
        "T_INT_LITERAL",
        "T_STRING_LITERAL",
        "T_REAL_LITERAL",
        "T_IDENTIFIER",
        "INT",
        "WHILE",
        "IF",
        "ELSE",
        "RETURN",
        "READ",
        "WRITE",
        "BEGIN",
        "END",
        "MAIN",
        "REAL",
    };
    if (token == -1) {
        return strdup("EOF");
    } else if (token < 256) {
        char str[2] = {token, 0};
        return strdup(str);
    } else {
        return strdup(token_strs[token - 256]);
    }
}