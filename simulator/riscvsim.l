%option yylineno
%option noyywrap

%{

#include <string.h>
#include "riscvsim.tab.h"
#include "defs.h"

%}

letter [@_.a-zA-Z]
digit  [0-9]
alpha  {letter}|{digit}

%%

[ \t\n"\r\n"]+ { /* skip */ }
"#".*    { /* skip */ }

.data   { return _DATA; }
.text   { return _TEXT; }

.word    { return _WORD; }

jal     { return _JAL; }
j       { return _J; }
ret     { return _RET; }

bge     { yylval.i = SIGNED_TYPE; return _BGE; }
bgeu    { yylval.i = UNSIGNED_TYPE; return _BGE; }
blt     { yylval.i = SIGNED_TYPE; return _BLT; }
bltu    { yylval.i = UNSIGNED_TYPE; return _BLT; }
bgt     { yylval.i = SIGNED_TYPE; return _BGT; }
bgtu    { yylval.i = UNSIGNED_TYPE; return _BGT; }
ble     { yylval.i = SIGNED_TYPE; return _BLE; }
bleu    { yylval.i = UNSIGNED_TYPE; return _BLE; }
bne     { yylval.i = NO_TYPE; return _BNE; }
beq     { yylval.i = NO_TYPE; return _BEQ; }

add     { return _ADD; }
addi    { return _ADDI; }
sub     { return _SUB; }
mv      { return _MV; }

lw      { return _LW; }
li      { return _LI; }
sw      { return _SW; }

nop     { return _NOP; }

zero  { yylval.i = 0; return _REGISTER; }
ra    { yylval.i = 1; return _REGISTER; }
sp    { yylval.i = 2; return _REGISTER; }
gp    { yylval.i = 3; return _REGISTER; }
tp    { yylval.i = 4; return _REGISTER; }
t0    { yylval.i = 5; return _REGISTER; }
t1    { yylval.i = 6; return _REGISTER; }
t2    { yylval.i = 7; return _REGISTER; }
fp    { yylval.i = 8; return _REGISTER; }
s0    { yylval.i = 8; return _REGISTER; }
s1    { yylval.i = 9; return _REGISTER; }
a0    { yylval.i = 10; return _REGISTER; }
a1    { yylval.i = 11; return _REGISTER; }
a2    { yylval.i = 12; return _REGISTER; }
a3    { yylval.i = 13; return _REGISTER; }
a4    { yylval.i = 14; return _REGISTER; }
a5    { yylval.i = 15; return _REGISTER; }
a6    { yylval.i = 16; return _REGISTER; }
a7    { yylval.i = 17; return _REGISTER; }
s2    { yylval.i = 18; return _REGISTER; }
s3    { yylval.i = 19; return _REGISTER; }
s4    { yylval.i = 20; return _REGISTER; }
s5    { yylval.i = 21; return _REGISTER; }
s6    { yylval.i = 22; return _REGISTER; }
s7    { yylval.i = 23; return _REGISTER; }
s8    { yylval.i = 24; return _REGISTER; }
s9    { yylval.i = 25; return _REGISTER; }
s10   { yylval.i = 26; return _REGISTER; }
s11   { yylval.i = 27; return _REGISTER; }
t3    { yylval.i = 28; return _REGISTER; }
t4    { yylval.i = 29; return _REGISTER; }
t5    { yylval.i = 30; return _REGISTER; }
t6    { yylval.i = 31; return _REGISTER; }

x0    { yylval.i = 0; return _REGISTER; }
x1    { yylval.i = 1; return _REGISTER; }
x2    { yylval.i = 2; return _REGISTER; }
x3    { yylval.i = 3; return _REGISTER; }
x4    { yylval.i = 4; return _REGISTER; }
x5    { yylval.i = 5; return _REGISTER; }
x6    { yylval.i = 6; return _REGISTER; }
x7    { yylval.i = 7; return _REGISTER; }
x8    { yylval.i = 8; return _REGISTER; }
x9    { yylval.i = 9; return _REGISTER; }
x10   { yylval.i = 10; return _REGISTER; }
x11   { yylval.i = 11; return _REGISTER; }
x12   { yylval.i = 12; return _REGISTER; }
x13   { yylval.i = 13; return _REGISTER; }
x14   { yylval.i = 14; return _REGISTER; }
x15   { yylval.i = 15; return _REGISTER; }
x16   { yylval.i = 16; return _REGISTER; }
x17   { yylval.i = 17; return _REGISTER; }
x18   { yylval.i = 18; return _REGISTER; }
x19   { yylval.i = 19; return _REGISTER; }
x20   { yylval.i = 20; return _REGISTER; }
x21   { yylval.i = 21; return _REGISTER; }
x22   { yylval.i = 22; return _REGISTER; }
x23   { yylval.i = 23; return _REGISTER; }
x24   { yylval.i = 24; return _REGISTER; }
x25   { yylval.i = 25; return _REGISTER; }
x26   { yylval.i = 26; return _REGISTER; }
x27   { yylval.i = 27; return _REGISTER; }
x28   { yylval.i = 28; return _REGISTER; }
x29   { yylval.i = 29; return _REGISTER; }
x30   { yylval.i = 30; return _REGISTER; }
x31   { yylval.i = 31; return _REGISTER; }


[+-]?{digit}+ { yylval.i = atol(yytext); return _NUMBER; }
{letter}{alpha}*":" { yylval.s = strdup(yytext); yylval.s[yyleng-1] = 0; return _LABEL_DEF; }
{letter}{alpha}*    { yylval.s = strdup(yytext); return _LABEL; }

","         { return _COMMA; }
"("         { return _LPAREN; }
")"         { return _RPAREN; }

. { parsererror("\nLEXICAL ERROR on character %c (line %d)", yytext[0], yylineno); }

%%