%top{
#include "parser.hpp"

#define YY_DECL yy::parser::symbol_type yylex()
}

%option noyywrap
%option yylineno
%option nodefault
%option nounput
%option noinput
%option warn
%option header-file="./build/lexer.hpp"
%option outfile="./build/lexer.cpp"

%{
#include <iostream>
#include <string>
#include <cerrno>
#include <cstdlib>

#include "ast/error.hpp"

static yy::location loc;

#define YY_USER_ACTION loc.columns(yyleng);

std::string process_string(const char* text) {
    std::string result;
    for (int i = 1; text[i+1] != '\0'; ++i) {
        if (text[i] == '\\') {
            i++; 
            switch(text[i]) {
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case 'r':  result += '\r'; break;
                case '0':  result += '\0'; break;
                case '\\': result += '\\'; break;
                case '\'': result += '\''; break;
                case '\"': result += '\"'; break;
                default:   result += text[i]; break; 
            }
        }
	else {
            result += text[i];
        }
    }
    return result;
}

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
                                  errno = 0;
                                  unsigned long x = std::strtoul(yytext, &end, 10);
                                  if(errno == ERANGE) {
                                      ErrorHandler::error(loc, "Integer constant is too large");
				      return yy::parser::make_YYEOF(loc);
                                  }
                                  if(*end != '\0') {    
                                      ErrorHandler::error(loc, "Illegal integer constant");
				      return yy::parser::make_YYEOF(loc);
                                  }
                                  return yy::parser::make_cint(x, loc);
                                }
{D}+\.{D}+([eE][\+\-]?{D}+)?    { std::string s(yytext);
                                  return yy::parser::make_re(std::stod(s), loc);
                                }
'([^\'\\]|{ESC})'               { return yy::parser::make_cchar(process_string(yytext).at(0), loc); }
\"([^\"\\\n]|{ESC})*\"          { return yy::parser::make_str(process_string(yytext), loc); }
\"([^\"\\\n]|{ESC})*            { std::cerr << "\033[91mError: Unterminated string\033[0m"
                                            << std::endl;
                                  std::exit(1);
                                }

{W}+                            { loc.step(); }
\n+                             { loc.lines(yyleng); loc.step(); } 

"(*"                { BEGIN(COMMENT); }
<COMMENT>"*)"       { BEGIN(INITIAL); }
<COMMENT>(.|\n)
<COMMENT><<EOF>>    {
                      ErrorHandler::error(loc, "Unclosed comment");
                      return yy::parser::make_YYEOF(loc);
                    }

. {
      ErrorHandler::error(loc, "Illegal character '" + std::string(1, yytext[0]) + "'");
  }

<<EOF>>    { return yy::parser::make_YYEOF(loc); }

%%
