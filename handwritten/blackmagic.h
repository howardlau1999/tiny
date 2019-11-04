#ifndef _BLACKMAGIC_H_
#define _BLACKMAGIC_H_


const int step = 2;
int ident = 0;

#define call(func)                                  \
    do {                                            \
        fprintf(tree, "%*s%s\n", ident, "", #func); \
        ident += step;                              \
        func();                                     \
        ident -= step;                              \
    } while (0)

#define terminal(expected, action)                     \
    do {                                               \
        if (accept(expected)) {                        \
            char* repr = print_token(token);           \
            fprintf(tree, "%*s%s\n", ident, "", repr); \
            free(repr);                                \
            ident += step;                             \
            action;                                    \
            token = lex();                             \
            ident -= step;                             \
        } else {                                       \
            parse_error(1, expected);                  \
        }                                              \
                                                       \
    } while (0)

#define optional(expected, action)                     \
    do {                                               \
        if (accept(expected)) {                        \
            action;                                    \
        }                                              \
    } while (0)

#define match_case(expected, action)                   \
    case expected: {                                   \
        terminal(expected, action);                    \
    } break                                            \

#endif