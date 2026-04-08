%top{
    // Include files here
}

%{
#include "lexer.hpp"
#include "parser.hpp" 
#define T_eof   0
%}

%x COMMENT

L   [a-zA-Z]
D   [0-9]
W   [ \t\n\r]
ESC \\([ntr0\\\'\"])

%option noyywrap
%option yylineno
%option nodefault

%%

"and"       { return T_and; }
"array"     { return T_array; }
"begin"     { return T_begin; }
"boolean"   { return T_bool; }
"char"      { return T_char; }
"dispose"   { return T_disp; }
"div"       { return T_div; }
"do"        { return T_do; }
"else"      { return T_else; }
"end"       { return T_end; }
"false"     { return T_false; }
"forward"   { return T_forw; }
"function"  { return T_func; }
"goto"      { return T_goto; }
"if"        { return T_if; }
"integer"   { return T_int; }
"label"     { return T_label; }
"mod"       { return T_mod; }
"new"       { return T_new; }
"nil"       { return T_nil; }
"not"       { return T_not; }
"of"        { return T_of; }
"or"        { return T_or; }
"procedure" { return T_proc; }
"program"   { return T_prog; }
"real"      { return T_real; }
"result"    { return T_res; }
"return"    { return T_ret; }
"then"      { return T_then; }
"true"      { return T_true; }
"var"       { return T_var; }
"while"     { return T_while; }

"<>"        { return T_neq; }
">="        { return T_geq; }
"<="        { return T_leq; }
":="        { return T_asgn; }

[\=\>\<\+\-\*\/\^\@\;\.\(\)\:\,\[\]]    { return yytext[0]; }

{L}({L}|{D}|_)*                 { return T_id; }
{D}+                            { return T_cint; }
{D}+\.{D}+([eE][\+\-]?{D}+)?    { return T_re; }
'([^\'\\]|{ESC})'               { return T_cchar; }
\"([^\"\\\n]|{ESC})*\"          { return T_str; }
{W}+                            { /*nothing*/ }

"(*"                { BEGIN(COMMENT); }
<COMMENT>"*)"       { BEGIN(INITIAL); }
<COMMENT>(.|\n)
<COMMENT><<EOF>>    { fprintf(stderr, "Error: Unclosed comment\n"); exit(1); }

. { fprintf(stderr, "illegal character '%c' (ASCII: %d) in line %d\n", yytext[0], yytext[0], yylineno); exit(1); }

%%
/*
int main() {
    int token;
    do {
        token = yylex();
        printf("token=%d, lexeme=\"%s\"\n", token, yytext);
    } while (token != T_eof);
    printf("Number of lines:%d\n", yylineno);
    return 0;
}
*/
