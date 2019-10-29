#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int match_keyword(const char* buf) {
    for (int i = 0; i < kw_size; ++i) {
        if (strcmp(buf, keywords[i]) == 0) {
            return T_INT + i;
        }
    }

    return T_IDENTIFIER;
}

extern FILE* source;
int line = 1;
int pos = 0;
int token_start = 1;
char* lval;

void lex_error(char* msg) {
    fprintf(stderr, "Line %d, Char %d: %s\n", line, pos, msg);
    exit(1);
}

void unrecognized_char(char c) {
    char buf[32] = "Unrecognized character: ?";
    buf[24] = c;
    lex_error(buf);
}

void next_line() { ++line, pos = 0; }
int next_char() {
    ++pos;
    int ch = fgetc(source);
    return ch;
}

int lex() {
    static int ch;
    static char buffer[256] = {0};
    static char* buf = buffer;
    while ((ch = next_char()) != EOF) {
        token_start = pos;
        switch (ch) {
            case '\t':
            case ' ':
                continue;

            case '\n':
                next_line();
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
                ch = next_char();

                if (ch == '*') {
                    while ((ch = next_char()) != EOF) {
                    comment:
                        if (ch == '\n') {
                            next_line();
                        } else if (ch == '*') {
                            ch = next_char();
                            if (ch == '/') break;
                            goto comment;
                        } else if (ch == EOF) {
                            lex_error("Unterminated comment");
                        }
                    }
                } else {
                    ungetc(ch, source);
                    --pos;
                    return '/';
                }
                break;

            case '"':
                *(buf++) = ch;
                while ((ch = next_char()) != EOF) {
                    *(buf++) = ch;
                    if (ch == '"') {
                        *buf = '\0';
                        lval = strdup(buf = buffer);
                        break;
                    } else if (ch == EOF || ch == '\n') {
                        lex_error("Unterminated string");
                    }
                }
                return T_STRING_LITERAL;
                break;

            case '0' ... '9': {
                int is_real = 0;
                *(buf++) = ch;
                while ((ch = next_char()) != EOF) {
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
                            --pos;
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
                while ((ch = next_char()) != EOF) {
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
                            --pos;
                            return match_keyword(buffer);
                    }
                }
                break;

            case '!':
                ch = next_char();
                if (ch == '=') return T_NE;
                unrecognized_char(ch);
                break;
            case '=':
                ch = next_char();
                if (ch == '=') return T_EQ;
                unrecognized_char(ch);
                break;
            case ':':
                ch = next_char();
                if (ch == '=') return T_ASSIGN;
                unrecognized_char(ch);
                break;
            default:
                unrecognized_char(ch);
                break;
        }
    }
}
