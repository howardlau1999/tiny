#include "token.h"
#include "blackmagic.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char* lval;
int lex();
int token;

extern int line;
extern int token_start;

FILE *source, *tree;

void Type();
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

int ww = 0, wtop = -1, wstack[100];

#define _BEG_WHILE \
    { wstack[++wtop] = ++ww; }
#define _END_WHILE \
    { wtop--; }
#define _w (wstack[wtop])

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

void match(int expected) { terminal(expected, {}); }

void Type() {
    switch (token) {
        match_case(T_INT, printf("INT "));
        match_case(T_REAL, printf("REAL "));
        default:
            parse_error(2, T_INT, T_REAL);
            break;
    }
}

void FormalParam() {
    call(Type);
    printf("PARAM ");
    terminal(T_IDENTIFIER, puts(lval));
}

void LocalVarDecl() {
    call(Type);
    printf("VAR ");
    terminal(T_IDENTIFIER, puts(lval));
    match(';');
}

void AssignStmt() {
    char* id;
    terminal(T_IDENTIFIER, id = strdup(lval));
    match(T_ASSIGN);
    call(Expression);
    match(';');
    printf("POP %s\n", id);
    free(id);
}

void ReturnStmt() {
    terminal(T_RETURN, {});
    call(Expression);
    terminal(';', {});
    puts("RET");
}

void WhileStmt() {
    terminal(T_WHILE, {
        _BEG_WHILE;
        printf("_begWhile_%d:\n", _w);
    });

    match('(');
    call(BoolExpression);
    match(')');
    printf("JZ _endWhile_%d\n", _w);
    call(Statement);
    printf("JMP _begWhile_%d\n", _w);

    printf("_endWhile_%d:\n", _w);
    _END_WHILE;
}

void IfStmt() {
    terminal(T_IF, {
        _BEG_IF;
        printf("_begIf_%d:\n", _i);
    });
    match('(');
    call(BoolExpression);
    match(')');
    printf("JZ _elIf_%d\n", _i);
    call(Statement);
    printf("JMP _endIf_%d\n_elIf_%d:\n", _i, _i);
    if (accept(T_ELSE)) {
        match(T_ELSE);
        call(Statement);
    }
    printf("_endIf_%d:\n", _i);
    _END_IF;
}

void WriteStmt() {
    char* filename;
    match(T_WRITE);
    match('(');
    call(Expression);
    match(',');
    terminal(T_STRING_LITERAL, filename = strdup(lval););
    match(')');
    match(';');
    printf("WRITE %s\n", filename);
    free(filename);
}

void ReadStmt() {
    char *filename, *id;
    match(T_READ);
    match('(');
    terminal(T_IDENTIFIER, id = strdup(lval););
    match(',');
    terminal(T_STRING_LITERAL, filename = strdup(lval););
    match(')');
    match(';');
    printf("READ %s\n", filename);
    printf("POP %s\n", id);
    free(filename);
    free(id);
}

void Num() {
    switch(token) {
        match_case(T_REAL_LITERAL, {});
        match_case(T_INT_LITERAL, {});
        default:
        parse_error(2, T_REAL_LITERAL, T_INT_LITERAL);
        break;
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
                match(',');
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
    switch (token) {
        match_case(T_EQ, action = "CMPEQ");
        match_case(T_NE, action = "CMPNE");
        match_case(T_LE, action = "CMPLE");
        match_case(T_GE, action = "CMPGE");
        match_case('<', action = "CMPLT");
        match_case('>', action = "CMPGT");
        default:
            parse_error(6, T_EQ, T_NE, T_LE, T_GE, '<', '>');
        break;
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
            match('(');
            call(Expression);
            match(')');
            break;
        case T_IDENTIFIER: {
            char* str;
            terminal(T_IDENTIFIER, str = strdup(lval));
            if (accept('(')) {
                match('(');
                call(ActualParams);
                match(')');
                printf("CALL %s\n", str);
            } else {
                printf("PUSH %s\n", str);
            }

            free(str);
        } break;
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
        case T_WHILE:
            call(WhileStmt);
            break;
        default:
            parse_error(9, T_BEGIN, T_INT, T_REAL, T_IDENTIFIER, T_RETURN, T_IF,
                        T_WRITE, T_READ, T_WHILE);
            break;
    }
}

void Block() {
    match(T_BEGIN);
    while (!accept(T_END)) call(Statement);
    match(T_END);
}

void FormalParams() {
    switch (token) {
        case T_INT:
        case T_REAL:
            call(FormalParam);
            while (accept(',')) {
                match(',');
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
    optional(T_MAIN, {
        match(T_MAIN);
        printf("ENTRY ");
    });
    printf("FUNC ");
    terminal(T_IDENTIFIER, puts(funcname = strdup(lval)););
    match('(');
    call(FormalParams);
    match(')');
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
