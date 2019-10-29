#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <stdio.h>

extern char* keywords[];
extern int kw_offset;
extern int kw_size;

typedef enum {
    T_EQ = 256,
    T_NE,
    T_ASSIGN,
    T_INT_LITERAL,
    T_STRING_LITERAL,
    T_REAL_LITERAL,
    T_IDENTIFIER,
    T_INT,
    T_WHILE,
    T_IF,
    T_ELSE,
    T_RETURN,
    T_READ,
    T_WRITE,
    T_BEGIN,
    T_END,
    T_MAIN,
    T_REAL,
} TokenType;

static void print_token(int token) {
    static char* token_strs[] = {
        "T_EQ",
        "T_NE",
        "T_ASSIGN",
        "T_INT_LITERAL",
        "T_STRING_LITERAL",
        "T_REAL_LITERAL",
        "T_IDENTIFIER",
        "T_INT",
        "T_WHILE",
        "T_IF",
        "T_ELSE",
        "T_RETURN",
        "T_READ",
        "T_WRITE",
        "T_BEGIN",
        "T_END",
        "T_MAIN",
        "T_REAL",
    };

    if (token < 256) {
        printf("%c ", token);
    } else {
        printf("%s ", token_strs[token - 256]);
    }
}

#endif  // _TOKEN_H_
