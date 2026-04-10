%top{
#include "parser.hpp"

#define YY_DECL yy::parser::symbol_type yylex()
}

%option noyywrap
%option yylineno
%option nodefault
%option nounput
%option warn
%option header-file="./build/lexer.hpp"

%{
#include <iostream>
#include <string>

static yy::location loc;

#define YY_USER_ACTION loc.columns(yyleng);
%}

%x COMMENT

L   [a-zA-Z]
D   [0-9]
W   [ \t\r]
ESC \\([ntr0\\\'\"])

%%

%{
loc.step();
%}

"and"       { return yy::parser::make_and(loc); }
"array"     { return yy::parser::make_array(loc); }
"begin"     { return yy::parser::make_begin(loc); }
"boolean"   { return yy::parser::make_bool(loc); }
"char"      { return yy::parser::make_char(loc); }
"dispose"   { return yy::parser::make_disp(loc); }
"div"       { return yy::parser::make_div(loc); }
"do"        { return yy::parser::make_do(loc); }
"else"      { return yy::parser::make_else(loc); }
"end"       { return yy::parser::make_end(loc); }
"false"     { return yy::parser::make_false(loc); }
"forward"   { return yy::parser::make_forw(loc); }
"function"  { return yy::parser::make_func(loc); }
"goto"      { return yy::parser::make_goto(loc); }
"if"        { return yy::parser::make_if(loc); }
"integer"   { return yy::parser::make_int(loc); }
"label"     { return yy::parser::make_label(loc); }
"mod"       { return yy::parser::make_mod(loc); }
"new"       { return yy::parser::make_new(loc); }
"nil"       { return yy::parser::make_nil(loc); }
"not"       { return yy::parser::make_not(loc); }
"of"        { return yy::parser::make_of(loc); }
"or"        { return yy::parser::make_or(loc); }
"procedure" { return yy::parser::make_proc(loc); }
"program"   { return yy::parser::make_prog(loc); }
"real"      { return yy::parser::make_real(loc); }
"result"    { return yy::parser::make_res(loc); }
"return"    { return yy::parser::make_ret(loc); }
"then"      { return yy::parser::make_then(loc); }
"true"      { return yy::parser::make_true(loc); }
"var"       { return yy::parser::make_var(loc); }
"while"     { return yy::parser::make_while(loc); }

"<>"        { return yy::parser::make_neq(loc); }
">="        { return yy::parser::make_geq(loc); }
"<="        { return yy::parser::make_leq(loc); }
":="        { return yy::parser::make_asgn(loc); }

[\=\>\<\+\-\*\/\^\@\;\.\(\)\:\,\[\]]    { return yy::parser::symbol_type(yytext[0], loc); }

{L}({L}|{D}|_)*                 { std::string s(yytext);
                                  return yy::parser::make_id(s, loc);
                                }
{D}+                            { char *end;
                                  unsigned long x = std::strtoul(yytext, &end, 10);
                                  if (*end != '\0') {
                                      std::cerr << "\033[91mError: Illegal integer constant\n"
                                                << std::endl;
                                      std::exit(1);
                                  }
                                  return yy::parser::make_cint(x, loc);
                                }
{D}+\.{D}+([eE][\+\-]?{D}+)?    { std::string s(yytext);
                                  return yy::parser::make_re(std::stod(s), loc);
                                }
'([^\'\\]|{ESC})'               { return yy::parser::make_cchar(yytext[1], loc); }
\"([^\"\\\n]|{ESC})*\"          { std::string s(yytext);
                                  return yy::parser::make_str(s, loc);
                                }
{W}+                            { loc.step(); }
\n+                             { loc.lines(yyleng); loc.step(); } 

"(*"                { BEGIN(COMMENT); }
<COMMENT>"*)"       { BEGIN(INITIAL); }
<COMMENT>(.|\n)
<COMMENT><<EOF>>    { std::cerr << "\033[91mError: Unclosed comment" << std::endl 
                                << "Line: " << loc.begin.line << "\033[0m"
                                << std::endl;
                      std::exit(1);
                    }

. { std::cerr << "\033[91mIllegal character '" << yytext[0]
              << "' at line " << loc.begin.line << "\033[0m"
              << std::endl;
    std::exit(1);
  }

<<EOF>>    { return yy::parser::make_YYEOF(loc); }

%%
