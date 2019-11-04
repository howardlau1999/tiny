#include "token.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char* lval;
int lex();
int token;

extern int line;
extern int token_start;

int ident = -2;
const int step = 2;

FILE *source, *tree;

void Type();
void Id();
void FormalParam();
void FormalParams();
void Main();
void Block();
void Statement();
void Expression();
void BoolExpression();

int ii = 0, itop = -1, istack[100];

#define _BEG_IF \
    { istack[++itop] = ++ii; }
#define _END_IF \
    { itop--; }
#define _i (istack[itop])

int accept(int expected) { return token == expected; }

void parse_error(int n, ...) {
    fprintf(stderr, "Line %d, Char %d: Expected ", line, token_start);
    va_list expected;
    va_start(expected, n);
    for (size_t i = 0; i < n; ++i) {
        if (i) {
            fprintf(stderr, ", ");
        }
        char* repr = print_token(va_arg(expected, int));
        fprintf(stderr, "%s", repr);
        free(repr);
    }
    va_end(expected);
    char* repr = print_token(token);
    fprintf(stderr, " Got %s\n", repr);
    free(repr);
    exit(1);
}

#define call(func)                                  \
    do {                                            \
        ident += step;                              \
        fprintf(tree, "%*s%s\n", ident, "", #func); \
        func();                                     \
        ident -= step;                              \
    } while (0)

#define terminal(expected, action)                                   \
    do {                                                             \
        if (accept(expected)) {                                      \
            ident += step;                                           \
            char *repr = print_token(token);                         \
            fprintf(tree, "%*s%s\n", ident, "", repr);               \
            free(repr);                                              \
            action;                                                  \
            token = lex();                                           \
            ident -= step;                                           \
        } else {                                                     \
            parse_error(1, expected);                                \
        }                                                            \
                                                                     \
    } while (0)

void Match(int expected) { terminal(expected, {}); }

void Type() {
    if (accept(T_INT)) {
        terminal(T_INT, printf("INT "));
    } else if (accept(T_REAL)) {
        terminal(T_REAL, printf("REAL "));
    } else {
        parse_error(2, T_INT, T_REAL);
    }
}

void Id() { terminal(T_IDENTIFIER, {}); }

void FormalParam() {
    call(Type);
    printf("PARAM ");
    terminal(T_IDENTIFIER, puts(lval));
}

void LocalVarDecl() {
    call(Type);
    printf("VAR ");
    terminal(T_IDENTIFIER, puts(lval));
    Match(';');
}

void AssignStmt() {
    char* id;
    terminal(T_IDENTIFIER, id = strdup(lval));
    Match(T_ASSIGN);
    call(Expression);
    Match(';');
    printf("POP %s\n", id);
    free(id);
}

void ReturnStmt() {
    terminal(T_RETURN, {});
    call(Expression);
    terminal(';', {});
    puts("RET");
}

void IfStmt() {
    terminal(T_IF, {
        _BEG_IF;
        printf("_begIf_%d:\n", _i);
    });
    Match('(');
    call(BoolExpression);
    Match(')');
    printf("JZ _elIf_%d\n", _i);
    call(Statement);
    printf("JMP _endIf_%d\n_elIf_%d:\n", _i, _i);
    if (accept(T_ELSE)) {
        Match(T_ELSE);
        call(Statement);
    }
    printf("_endIf_%d:\n", _i);
    _END_IF;
}

void WriteStmt() {
    char* filename;
    Match(T_WRITE);
    Match('(');
    call(Expression);
    Match(',');
    terminal(T_STRING_LITERAL, filename = strdup(lval););
    Match(')');
    Match(';');
    printf("WRITE %s\n", filename);
    free(filename);
}

void ReadStmt() {
    char *filename, *id;
    Match(T_READ);
    Match('(');
    terminal(T_IDENTIFIER, id = strdup(lval););
    Match(',');
    terminal(T_STRING_LITERAL, filename = strdup(lval););
    Match(')');
    Match(';');
    printf("READ %s\n", filename);
    printf("POP %s\n", id);
    free(filename);
    free(id);
}

void Num() {
    if (accept(T_REAL_LITERAL)) {
        Match(T_REAL_LITERAL);
    } else if (accept(T_INT_LITERAL)) {
        Match(T_INT_LITERAL);
    } else {
        parse_error(2, T_REAL_LITERAL, T_INT_LITERAL);
    }
}

void ActualParams() {
    switch (token) {
        case T_INT_LITERAL:
        case T_REAL_LITERAL:
        case '(':
        case T_IDENTIFIER:
            call(Expression);
            while (accept(',')) {
                Match(',');
                call(Expression);
            }
            break;

        default:
            break;
    }
}

void BoolExpression() {
    char* action;
    call(Expression);
    if (accept(T_EQ)) {
        terminal(T_EQ, action = "CMPEQ";);
    } else if (accept(T_NE)) {
        terminal(T_NE, action = "CMPNE";);
    } else {
        parse_error(2, T_EQ, T_NE);
    }
    call(Expression);
    puts(action);
}

void PrimaryExpr() {
    switch (token) {
        case T_REAL_LITERAL:
            terminal(T_INT_LITERAL, printf("PUSH %s\n", lval));
            break;
        case T_INT_LITERAL:
            terminal(T_INT_LITERAL, printf("PUSH %s\n", lval));
            break;
        case '(':
            Match('(');
            call(Expression);
            Match(')');
            break;
        case T_IDENTIFIER: {
            char* str;
            terminal(T_IDENTIFIER, str = strdup(lval));
            if (accept('(')) {
                Match('(');
                call(ActualParams);
                Match(')');
                printf("CALL %s\n", str);
            } else {
                printf("PUSH %s\n", str);
            }
            
            free(str);
        }
            break;
        default:
            parse_error(4, T_REAL_LITERAL, T_INT_LITERAL, '(', T_IDENTIFIER);
            break;
    }
}

void MultiplicativeExpr() {
    call(PrimaryExpr);
    char* action;
    while (1) {
        if (accept('*')) {
            terminal('*', action = "MUL");
            call(PrimaryExpr);
            puts(action);
        } else if (accept('/')) {
            terminal('/', action = "DIV");
            call(PrimaryExpr);
            puts(action);
        } else {
            break;
        }
    }
}

void Expression() {
    call(MultiplicativeExpr);
    char* action;
    while (1) {
        if (accept('+')) {
            terminal('+', action = "ADD");
            call(MultiplicativeExpr);
            puts(action);
        } else if (accept('-')) {
            terminal('-', action = "SUB");
            call(MultiplicativeExpr);
            puts(action);
        } else {
            break;
        }
    }
}

void Statement() {
    switch (token) {
        case T_BEGIN:
            call(Block);
            break;
        case T_INT:
        case T_REAL:
            call(LocalVarDecl);
            break;
        case T_IDENTIFIER:
            call(AssignStmt);
            break;
        case T_RETURN:
            call(ReturnStmt);
            break;
        case T_IF:
            call(IfStmt);
            break;
        case T_WRITE:
            call(WriteStmt);
            break;
        case T_READ:
            call(ReadStmt);
            break;
        default:
            parse_error(8, T_BEGIN, T_INT, T_REAL, T_IDENTIFIER, T_RETURN, T_IF,
                        T_WRITE, T_READ);
            break;
    }
}

void Block() {
    Match(T_BEGIN);
    while (!accept(T_END)) call(Statement);
    Match(T_END);
}

void FormalParams() {
    switch (token) {
        case T_INT:
        case T_REAL:
            call(FormalParam);
            while (accept(',')) {
                Match(',');
                call(FormalParam);
            }
            break;
        case ')':
            break;
        default:
            parse_error(3, T_INT, T_REAL, ')');
            break;
    }
}

void MethodDecl() {
    char* funcname;
    call(Type);
    if (accept(T_MAIN)) {
        Match(T_MAIN);
        printf("ENTRY ");
    }
    printf("FUNC ");
    terminal(T_IDENTIFIER, puts(funcname = strdup(lval)););
    Match('(');
    call(FormalParams);
    Match(')');
    call(Block);
    printf("END FUNC %s\n", funcname);
    free(funcname);
}

void Program() {
    call(MethodDecl);
    while (1) {
        if (accept(T_INT) || accept(T_REAL)) {
            call(MethodDecl);
        } else if (accept(-1)) {
            return;
        } else {
            parse_error(3, T_INT, T_REAL, -1);
        }
    }
}

int parse() {
    token = lex();
    call(Program);
}

int main(int argc, char* argv[]) {
    source = stdin;
    tree = fopen("tree.txt", "w");
    parse();
    fclose(tree);
}
