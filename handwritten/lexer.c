#include "token.h"

#include <string.h>

int match_keyword(const char* buf) {
    for (int i = 0; i < kw_size; ++i) {
        if (strcmp(buf, keywords[i]) == 0) {
            return T_INT + i;
        }
    }

    return T_IDENTIFIER;
}

typedef enum {
    S_INITIAL,
    S_STRING,
    S_COMMENT,
    S_NE_BEGIN,
    S_EQ_BEGIN,
    S_ASSIGN_BEGIN,
} State;

extern FILE* source;
char* lval;
int __lex() {
    static size_t line = 1;
    static int ch;
    static int state = S_INITIAL;
    static char buffer[256] = {0};
    static char* buf = buffer;
    while ((ch = fgetc(source)) != EOF) {
        switch (ch) {
            case '\t':
            case ' ':
                continue;

            case '\n':
                ++line;
                break;

            case '(':
            case ')':
            case ',':
            case ';':
            case '+':
            case '-':
            case '*':
                return ch;
                break;

            case '/':
                ch = fgetc(source);
                if (ch == '*') {
                    while ((ch = fgetc(source)) != EOF) {
                    comment:
                        if (ch == '*') {
                            ch = fgetc(source);
                            if (ch == '/') break;
                            if (ch == '\n') {
                                ++line;
                                break;
                            }
                            goto comment;
                        }
                    }
                } else {
                    ungetc(ch, source);
                    return '/';
                }
                break;

            case '"':
                *(buf++) = ch;
                while ((ch = fgetc(source)) != EOF) {
                    *(buf++) = ch;
                    if (ch == '"') {
                        *buf = 0;
                        lval = strdup(buf = buffer);
                        break;
                    }
                }
                return T_STRING_LITERAL;
                break;

            case '0' ... '9': {
                int is_real = 0;
                *(buf++) = ch;
                while ((ch = fgetc(source)) != EOF) {
                    switch (ch) {
                        case '.':
                            is_real = 1;
                        case '0' ... '9':
                            *(buf++) = ch;
                            break;

                        default:
                            *buf = '\0';
                            lval = strdup(buf = buffer);
                            ungetc(ch, source);
                            if (is_real) {
                                return T_REAL_LITERAL;
                            } else {
                                return T_INT_LITERAL;
                            }
                            
                        break;
                    }
                }
            }

            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_':
                *(buf++) = ch;
                while ((ch = fgetc(source)) != EOF) {
                    switch (ch) {
                        case '0' ... '9':
                        case 'a' ... 'z':
                        case 'A' ... 'Z':
                        case '_':
                            *(buf++) = ch;
                            break;

                        default:
                            *buf = '\0';
                            lval = strdup(buf = buffer);
                            ungetc(ch, source);
                            return match_keyword(buffer);
                    }
                }
                break;

            case '!':
                ch = fgetc(source);
                if (ch == '=') return T_NE;
                break;
            case '=':
                ch = fgetc(source);
                if (ch == '=') return T_EQ;
                break;
            case ':':
                ch = fgetc(source);
                if (ch == '=') return T_ASSIGN;
                break;
        }
    }
}

