%{
#define YYSTYPE char *
#include "y.tab.h"
int cur_line_num = 1;
void yyerror(const char *msg);
void unrecognized_char(char c);
void unterminated_string();
#define _DUPTEXT {yylval = strdup(yytext);}
%}

INTEGER             ([0-9]+)
REAL  	            ([0-9]+\.[0-9]+)
IDENTIFIER          ([_a-zA-Z][_a-zA-Z0-9]*)
OPERATOR            ([+*-/])
SEPARATOR           ([;,()])
UNTERM_STRING       (\042[^\042\n]*)
STRING              (\042[^\042\n]*\042)
%x COMMENT

%%

"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>(.|\n) ;
<COMMENT><<EOF>> { yyerror("Unexpected file end (unterminated comment).\n"); }

[\n]                { cur_line_num++;                       }
[ \t\r\a]+          {                                       }

{OPERATOR}          { return yytext[0];                     }
{SEPARATOR}         { return yytext[0];                     }

":="                { return T_ASSIGN;                      }
"!="                { return T_NE;                          }
"=="                { return T_EQ;                          }

"IF"                { return T_IF;                          }
"ELSE"              { return T_ELSE;                        }
"WRITE"             { return T_WRITE;                       }
"READ"              { return T_READ;                        }
"RETURN"            { return T_RETURN;                      }
"BEGIN"             { return T_BEGIN;                       }
"END"               { return T_END;                         }
"MAIN"              { return T_MAIN;                        }
"INT"               { return T_INT;                         }
"REAL"              { return T_REAL;                        }

{INTEGER}           { _DUPTEXT return T_INT_LITERAL;        }
{STRING}            { _DUPTEXT return T_STRING_LITERAL;     }
{IDENTIFIER}        { _DUPTEXT return T_IDENTIFIER;         }
{REAL}              { _DUPTEXT return T_REAL_LITERAL;       }

<<EOF>>             { return 0;                             }

{UNTERM_STRING}     { unterminated_string();  }
.                   { unrecognized_char(yytext[0]);         }

%%

int yywrap(void) { 
    return 1;
}

void unrecognized_char(char c) {
    char buf[32] = "Unrecognized character: ?";
    buf[24] = c;
    yyerror(buf);
}

void unterminated_string() {
    yyerror("Unterminate string constant");
}

void yyerror(const char *msg) {
    fprintf(stderr, "Error at line %d:\n\t%s\n", cur_line_num, msg);
    exit(-1);
}