#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
extern char* lval;
int lex();
int token;

extern int line;
extern int token_start;

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

#define _BEG_IF     {istack[++itop] = ++ii;}
#define _END_IF     {itop--;}
#define _i          (istack[itop])


int accept(int expected) {
    return token == expected;
}


void parse_error(int n, ...) {
    fprintf(stderr, "Line %d, Char %d: Expected ", line, token_start);
    va_list expected;
    va_start(expected, n);
    for (size_t i = 0; i < n; ++i) {
        if (i) {
           fprintf(stderr, ", ");
        }
        fprintf(stderr, "%s", print_token(va_arg(expected, int)));
    }
    va_end(expected);
    fprintf(stderr, " Got %s\n", print_token(token));
    exit(1);
}

#define terminal(expected, action) do {\
    if (accept(expected)) {\
        action\
        token = lex();\
    } else {\
        parse_error(1, expected);\
    }\
    \
} while (0)

void Match(int expected) {
    terminal(expected, {});
}

void Type() {
    if (accept(T_INT)) {
        terminal(T_INT, 
            printf("INT ");
        );
    } else if (accept(T_REAL)) {
        terminal(T_REAL, 
            printf("REAL ");
        );
    } else parse_error(2, T_INT, T_REAL);
}

void Id() { terminal(T_IDENTIFIER, {}); }

void FormalParam() {
    Type();
    printf("PARAM ");
    terminal(T_IDENTIFIER,
        puts(lval);
    );
}

void LocalVarDecl() {
    Type();
    printf("VAR ");
    terminal(T_IDENTIFIER,
        puts(lval);
    );
    terminal(';', {});
}

void AssignStmt() {
    char * id;
    terminal(T_IDENTIFIER, 
        id = strdup(lval);
    );
    terminal(T_ASSIGN, {});
    Expression();
    terminal(';', {});
    printf("POP %s\n", id);
}

void ReturnStmt() {
    terminal(T_RETURN, {});
    Expression();
    terminal(';', {});
    puts("RET");
}

void IfStmt() {
    terminal(T_IF, {
        _BEG_IF; printf("_begIf_%d:\n", _i); 
    });
    Match('(');
    BoolExpression();
    Match(')');
    printf("JZ _elIf_%d\n", _i);
    Statement();
    printf("JMP _endIf_%d\n_elIf_%d:\n", _i, _i); 
    if (token == T_ELSE) {
        token = lex();
        Statement();
    }
    printf("_endIf_%d:\n", _i); _END_IF;
}

void String() {
    if (accept(T_STRING_LITERAL)) {
        token = lex();
    } else {
        parse_error(1, T_STRING_LITERAL);
    }
}

void WriteStmt() {
    char * filename;
    Match(T_WRITE);
    Match('(');
    Expression();
    Match(',');
    terminal(T_STRING_LITERAL, 
        filename = strdup(lval);
    );
    Match(')');
    Match(';');
    printf("WRITE %s\n", filename);
}

void ReadStmt() {
    char * filename, * id;
    Match(T_READ);
    Match('(');
    terminal(T_IDENTIFIER, 
        id = strdup(lval);
    );
    Match(',');
    terminal(T_STRING_LITERAL, 
        filename = strdup(lval);
    );
    Match(')');
    Match(';');
    printf("READ %s\n", filename);
    printf("POP %s\n", id);
}

void Num() {
    if (accept(T_REAL_LITERAL) || accept(T_INT_LITERAL)) {
        token = lex();
    } else {
        parse_error(2, T_REAL_LITERAL, T_INT_LITERAL);
    }
}

void ActualParams() {
    switch (token)
    {
    case T_INT_LITERAL:
    case T_REAL_LITERAL:
    case '(':
    case T_IDENTIFIER:
        Expression();
        while (accept(',')) {
            token = lex();
            Expression();
        }
        break;
    
    default:
        break;
    }
}

void BoolExpression() {
    char * action;
    Expression();
    if (accept(T_EQ)) {
        terminal(T_EQ, 
            action = "CMPEQ";
        );
    } else if (accept(T_NE)) {
        terminal(T_NE, 
            action = "CMPNE";
        );
    } else {
        parse_error(2, T_EQ, T_NE);
    }
    Expression();
    puts(action);
}

void PrimaryExpr() {
    switch (token)
    {
    case T_REAL_LITERAL:
    case T_INT_LITERAL:
        printf("PUSH %s\n", lval);
        token = lex();
        break;
    case '(':
        Match('(');
        Expression();
        Match(')');
    case T_IDENTIFIER:
        {
            char * str = strdup(lval);
            token = lex();
            if (accept('(')) {
                Match('(');
                ActualParams();
                Match(')');
                printf("CALL %s\n", str);
            } else {
                printf("PUSH %s\n", str);
            }
        }
    default:
        break;
    }
}

void MultiplicativeExpr() {
    PrimaryExpr();
    char * action;
    while (1) {
        if (accept('*')) {
            terminal('*', 
                action = "MUL";
            );
            PrimaryExpr();
            puts(action);
        } else if (accept('/')) {
            terminal('/', 
                action = "DIV";
            );
            PrimaryExpr();
            puts(action);
        } else {
            break;
        }
    }
}

void Expression() {
    MultiplicativeExpr();
    char * action;
    while (1) {
        if (accept('+')) {
            terminal('+', 
                action = "ADD";
            );
            MultiplicativeExpr();
            puts(action);
        } else if (accept('-')) {
            terminal('-', 
                action = "SUB";
            );
            MultiplicativeExpr();
            puts(action);
        } else {
            break;
        }
    }
    
}

void Statement() {
    switch (token) {
        case T_BEGIN:
            Block();
            break;
        case T_INT:
        case T_REAL:
            LocalVarDecl();
            break;
        case T_IDENTIFIER:
            AssignStmt();
            break;
        case T_RETURN:
            ReturnStmt();
            break;
        case T_IF:
            IfStmt();
            break;
        case T_WRITE:
            WriteStmt();
            break;
        case T_READ:
            ReadStmt();
            break;
        default:
            parse_error(8, T_BEGIN, T_INT, T_REAL, T_IDENTIFIER, T_RETURN, T_IF, T_WRITE, T_READ);
            break;
    }
}

void Block() {
    Match(T_BEGIN);
    while (!accept(T_END)) Statement();
    Match(T_END);
}

void FormalParams() {
    switch (token) {
        case T_INT:
        case T_REAL:
            FormalParam();
            while (accept(',')) {
                Match(',');
                FormalParam();
            }
            break;
        case ')':
            break;
        default:
            parse_error(3, T_INT, T_REAL, ')');
            break;
    }

}

void Main() {
    if (accept(T_MAIN)) {
        printf("ENTRY ");
        token = lex();
    }
}

void MethodDecl() {
    char * funcname;
    Type();
    Main();
    printf("FUNC ");
    terminal(T_IDENTIFIER, 
        puts(funcname = strdup(lval));
    );
    Match('(');
    FormalParams();
    Match(')');
    Block();
    printf("END FUNC %s\n", funcname);
}

void Program() {
    MethodDecl();
    while (1) {
        if (accept(T_INT) || accept(T_REAL)) {
            MethodDecl();
        } else if (accept(-1)) {
            return;
        } else {
            parse_error(3, T_INT, T_REAL, -1);
        }
    }
}

int parse() {
    token = lex();
    Program();
}

FILE* source;

int main(int argc, char* argv[]) {
    source = stdin;
    parse();
}