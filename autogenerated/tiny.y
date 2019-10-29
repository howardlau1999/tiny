%{
#include <stdio.h>
#include <stdlib.h>
void yyerror(const char *);
#define YYSTYPE char*
int ii = 0, itop = -1, istack[100];

#define _BEG_IF     {istack[++itop] = ++ii;}
#define _END_IF     {itop--;}
#define _i          (istack[itop])
%}

%token T_EQ T_NE T_INT_LITERAL T_STRING_LITERAL T_REAL_LITERAL T_IDENTIFIER T_INT T_WHILE T_IF T_ELSE T_RETURN T_READ T_WRITE T_BEGIN T_END T_ASSIGN T_MAIN T_REAL

%left '+' '-'
%left '*' '/'

%%

Program:
    /* empty */ {}
|   Program MethodDecl {}
;

MethodDecl: Type FuncName '(' FormalParams ')' Block {
    printf("END FUNC %s\n", $3);
};

FormalParams: /* empty */ {} | FormalParam {} ;

FormalParam: Type T_IDENTIFIER {
    printf("PARAM %s\n", $2);
} | FormalParam ',' Type T_IDENTIFIER {
    printf("PARAM %s\n", $4);
};

FuncName: T_MAIN T_IDENTIFIER {
    printf("ENTRY FUNC %s\n", $2);
} | T_IDENTIFIER {
    printf("FUNC %s\n", $1);
};

Type: T_INT {
    printf("INT ");
} | T_REAL {};

Block: T_BEGIN Statements T_END {};

Statements: /* empty */ {} | Statements Statement {};

Statement: Block {} | LocalVarDecl {} | AssignStmt {} | ReturnStmt {} | IfStmt {} | WriteStmt {} | ReadStmt {};

LocalVarDecl: Type T_IDENTIFIER ';' {
    printf("VAR %s\n", $2);
};

AssignStmt: T_IDENTIFIER T_ASSIGN Expression ';' {
    printf("POP %s\n", $1);
};
ReturnStmt: T_RETURN Expression ';' {
    printf("RET\n");
};

IfStmt: If '(' BoolExpression ')' Then Statement EndThen EndIf {} | If '(' BoolExpression ')' Then Statement EndThen T_ELSE Statement EndIf {};

If: T_IF {
    _BEG_IF; printf("_begIf_%d:\n", _i); 
};

Then: {
    printf("JZ _elIf_%d\n", _i);
};

EndThen: { 
    printf("JMP _endIf_%d\n_elIf_%d:\n", _i, _i); 
};

EndIf: {
    printf("_endIf_%d:\n", _i); _END_IF;
};

WriteStmt: T_WRITE '(' Expression ',' T_STRING_LITERAL ')' ';' {
    printf("WRITE %s\n", $5);
};

ReadStmt: T_READ '(' T_IDENTIFIER ',' T_STRING_LITERAL ')' ';' {
    printf("READ %s\n", $5);
    printf("POP %s\n", $3);
};

Expression: Expression '+' Expression {
    printf("ADD\n");
} 
| Expression '-' Expression {
    printf("SUB\n");
} 
| Expression '*' Expression {
    printf("MUL\n");
} 
| Expression '/' Expression {
    printf("DIV\n");
} 
| Num {
    printf("PUSH %s\n", $1);
} 
| T_IDENTIFIER {
    printf("PUSH %s\n", $1);
} 
| '(' Expression ')' {} 
| T_IDENTIFIER '(' ActualParams ')' {
    printf("CALL %s\n", $1);
};

Num: T_INT_LITERAL {} | T_REAL_LITERAL {};
BoolExpression: Expression T_EQ Expression {
    printf("CMPEQ\n");
} | Expression T_NE Expression {
    printf("CMPNE\n");
};
ActualParams: /* empty */ {} | Expression ActualParam {};
ActualParam: /* empty */ {} | ActualParam ',' Expression {};

%%

int main() {
    return yyparse();
}