%option yylineno
%pointer

%{
    #include <stdlib.h>
    #include <errno.h>
    #include <limits.h>
    #include "ast.hpp"
    #include "parser.hpp"
    void yyerror(const char *);
%}


DIGIT    [0-9]*
ID       [a-zA-Z][a-zA-Z0-9]*

%x comment

%%
        int line_num = 1;

"/*"         BEGIN(comment);

<comment>[^*\n]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>\n             ++line_num;
<comment><<EOF>>        yyerror("dangling comment");
<comment>"*"+"/"        BEGIN(INITIAL);


"print"     { return T_PRINT; }
"return"    { return T_RETURN; }
"new"       { return T_NEW; }
"extends"   { return T_EXTEND; }
"if"        { return T_IF; }
"else"      { return T_ELSE; }
"while"     { return T_WHILE; }
"do"        { return T_DO; }

"->"        { return T_ARROW; }
","         { return T_COMMA; }
"."         { return T_DOT; }
"{"         { return T_LB; }
"}"         { return T_RB; }
"("         { return T_LP; }
")"         { return T_RP; }
"="         { return T_ASSIGN; }

"integer"   { return T_INT; }
"boolean"   { return T_BOOL; }
"none"      { return T_NONE; }
"true"      { return T_TRUE; }
"false"     { return T_FALSE; }

"or"        { return T_OR; }
"and"       { return T_AND; }
"equals"    { return T_EQUAL; }
">="        { return T_GREATOREQUAL; }
">"         { return T_GREATER; }
"+"         { return T_PLUS; }
"-"         { return T_MINUS; }

"*"         { return T_MULTIPLY; }
"/"         { return T_DIVIDE; }
"not"       { return T_NOT; }
";"         { return T_SEMICOLON; }

{ID}        { yylval.identifier_ptr = new IdentifierNode(yytext); return T_ID; }
{DIGIT}     { yylval.integer_ptr = new IntegerNode(atoi(yytext)); return T_NUMBER; }

[ \t\n]+    
.           { yyerror("invalid character"); }

%%

int yywrap(void) {
  return 1;
}
