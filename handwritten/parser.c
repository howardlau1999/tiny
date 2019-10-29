#include <stdlib.h>
#include <string.h>
#include "token.h"
extern char* lval;
int __lex();
int token;

void Match();
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

int lex() {
    int token = __lex();
    // print_token(token);
    return token;
}

int accept(int expected) {
    return token == expected;
}

void parse_error(const char* message) {
    printf("%s", message);
    print_token(token);
    puts("");
    exit(1);
}

#define terminal(expected, action) do {\
    if (accept(expected)) {\
        action\
        token = lex();\
    } else {\
        printf("Match ");\
        print_token(expected);\
        parse_error("got ");\
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
    } else parse_error("Type ");
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
        parse_error("String ");
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
        parse_error("Num ");
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
            action = "CMPEQ";
        );
    } else {
        parse_error("BoolExpression ");
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
            parse_error("Statement ");
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
            // Match(')');
            break;
        default:
            parse_error("FormalParams ");
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
        } else {
            break;
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