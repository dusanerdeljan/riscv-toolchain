%option noyywrap yylineno
%{
   #include <string.h>
   #include "micko.tab.h"
   #include "defs.h"
   extern YYSTYPE yylval;
%}

%%

[ \t\n"\r\n"]+       { /* skip */ }

"int"                { yylval.i = INT;  return _TYPE; }
"unsigned"           { yylval.i = UINT; return _TYPE; }
"void"               { yylval.i = VOID; return _TYPE; }
"if"                 { return _IF; }
"else"               { return _ELSE; }
"return"             { return _RETURN; }

"select"             { return _SELECT; }
"from"               { return _FROM; }
"where"              { return _WHERE; }
","                  { return _COMMA; }

"++"                 { return _POST_INC; }

"("                  { return _LPAREN; }
")"                  { return _RPAREN; }
"{"                  { return _LBRACKET; }
"}"                  { return _RBRACKET; }
";"                  { return _SEMICOLON; }
"="                  { return _ASSIGN; }

"para"               { return _PARA; }
":"                  { return _COLON; }

"["                  { return _LSQBRACKET; }
"]"                  { return _RSQBRACKET; }
"->"                 { return _ARROW; }
"branch"             { return _BRANCH; }
"first"              { return _FIRST; }
"second"             { return _SECOND; }
"third"              { return _THIRD; }
"otherwise"          { return _OTHERWISE; }

"while"              { return _WHILE; }

"?"                  { return _QUESTION; }

"and"                { yylval.i = AND; return _LOGOP; }
"or"                 { yylval.i = OR; return _LOGOP; }

"+"                  { yylval.i = ADD; return _AROP; }
"-"                  { yylval.i = SUB; return _AROP; }

"<"                  { yylval.i = LT; return _RELOP; }
">"                  { yylval.i = GT; return _RELOP; }
"<="                 { yylval.i = LE; return _RELOP; }
">="                 { yylval.i = GE; return _RELOP; }
"=="                 { yylval.i = EQ; return _RELOP; }
"!="                 { yylval.i = NE; return _RELOP; }

[a-zA-Z][a-zA-Z0-9]* { yylval.s = strdup(yytext); 
                       return _ID; }
[+-]?[0-9]{1,10}     { yylval.s = strdup(yytext); 
                       return _INT_NUMBER;}
[0-9]{1,10}[uU]      { yylval.s = strdup(yytext); 
                       yylval.s[yyleng-1] = 0;
                       return _UINT_NUMBER;}

\/\/.*               { /* skip */ }
"/*"([^*]|"\n"|(\*+([^*/]|"\n")))*"*"+"/" { /* skip */ }
.                    { printf("line %d: LEXICAL ERROR on char %c\n", yylineno, *yytext);}
%%
