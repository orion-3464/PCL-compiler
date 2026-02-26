%top{
    // Include files here
}

%{
    #define T_eof   0
    #define T_and   1001
    #define T_array 1002
    #define T_begin 1003
    #define T_bool  1004
    #define T_char  1005
    #define T_disp  1006
    #define T_div   1007
    #define T_do    1008
    #define T_else  1009
    #define T_end   1010
    #define T_false 1011
    #define T_forw  1012
    #define T_func  1013
    #define T_goto  1014
    #define T_if    1015
    #define T_int   1016
    #define T_label 1017
    #define T_mod   1018
    #define T_new   1019
    #define T_nil   1020
    #define T_not   1021
    #define T_of    1022
    #define T_or    1023
    #define T_proc  1024
    #define T_prog  1025
    #define T_real  1026
    #define T_res   1027
    #define T_ret   1028
    #define T_then  1029
    #define T_true  1030
    #define T_var   1031
    #define T_while 1032

    #define T_id    1033
    #define T_cint  1034
    #define T_re    1035
    #define T_cchar 1036
    #define T_str   1037

    #define T_neq   1038
    #define T_geq   1039
    #define T_leq   1040
    #define T_asgn  1041
%}

%x COMMENT

L   [a-zA-Z]
D   [0-9]
W   [ \t\n\r]
ESC \\([ntr0\\\'\"])

%option outfile="lexer.cpp"
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
0*{D}+                          { return T_cint; }
{D}+\.{D}+([eE][\+\-]?{D}+)?    { return T_re; }
'([^\"\'\\]|{ESC})'             { return T_cchar; }
\"([^\"\\\n]|{ESC})*\"             { return T_str; }
{W}+                            { /*nothing*/ }

"(*"                { BEGIN(COMMENT); }
<COMMENT>"*)"       { BEGIN(INITIAL); }
<COMMENT>\n         { /* newlines are handled by yylineno */ }
<COMMENT>.          { /* ignore any other character inside the comment */ }
<COMMENT><<EOF>>    { fprintf(stderr, "Error: Unclosed comment\n"); exit(1); }

. { fprintf(stderr, "illegal character\n"); exit(1); }

%%

int main() {
    int token;
    do {
        token = yylex();
        printf("token=%d, lexeme=\"%s\"\n", token, yytext);
    } while (token != T_eof);
    return 0;
}