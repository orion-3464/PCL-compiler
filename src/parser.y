%require "3.8"
%language "c++"
%locations
%verbose
%define api.location.file "location.hpp"

%code top {
    #include <cstdio>
    #include <cstdlib>
}

%code requires {
    
    #include "ast.hpp"
}

%code {
    #include "lexer.hpp"
    YY_DECL;
}

%define api.value.type variant
%define api.token.constructor
/*%define api.value.automove*/
%define parse.assert
%define parse.error verbose
%define api.token.prefix {T_}

%token and   "and"
%token array "array"
%token begin "begin"
%token bool  "boolean"
%token char  "char"
%token disp  "dispose"
%token div   "div"
%token do    "do"
%token else  "else"
%token end   "end"
%token false "false"
%token forw  "forward"
%token func  "function"
%token goto  "goto"
%token if    "if"
%token int   "integer"
%token label "label"
%token mod   "mod"
%token new   "new"
%token nil   "nil"
%token not   "not"
%token of    "of"
%token or    "or"
%token proc  "procedure"
%token prog  "program"
%token real  "real"
%token res   "result"
%token ret   "return"
%token then  "then"
%token true  "true"
%token var   "var"
%token while "while"
                        
%token id    
%token cint  
%token re    
%token cchar 
%token str

%token neq  "<>" 
%token geq  ">=" 
%token leq  "<=" 
%token asgn ":="

%nonassoc '=' '>' '<' "<=" ">=" "<>"
%left '+' '-' "or"
%left '*' '/' "div" "mod" "and"
%precedence UNARY
%precedence '^'
%precedence '@'
%precedence '['

%expect 1 /* dangling else */
   
%%

program:
    "program" id ';' body '.'
;

body:
    locals block
;

locals:
    %empty
|   locals local
;

local:
    "var" definitions
|   "label" id ids ';'
|   header ';' body ';'
|   "forward" header ';'
;

definitions:
    definition
|   definitions definition
;

definition:
    id ids ':' type ';'
;

ids:
    %empty
|   ids ',' id
;

header:
    "procedure" id '(' parameters ')' /* parameters are optional */
|   "function" id '(' parameters ')' ':' type
;

parameters:
    %empty
|   formal formals
;

formals:
    %empty
|   formals ';' formal
;

formal:
    opvar id ids ':' type
;

opvar:
    %empty
|   "var"
;

type:
    "integer"
|   "real"
|   "boolean"
|   "char"
|   "array" size "of" type
|   '^' type
;

size:
    %empty
|   '[' cint ']'
;

block:
    "begin" stmt stmts "end"   
;

stmts:
    %empty
|   stmts ';' stmt
;

stmt:
    %empty
|   expr ":=" expr
|   block
|   call
|   "if" expr "then" stmt elseop
|   "while" expr "do" stmt
|   id ':' stmt
|   "goto" id
|   "return"
|   "new" exprop expr
|   "dispose" brackop expr
;

elseop:
    %empty
|   "else" stmt
;

exprop:
    %empty
|   '[' expr ']'
;

brackop:
    %empty
|   '['']'
;

expr:
    id
|   "result"
|   str
|   expr '[' expr ']'
|   expr '^'              %prec '^'
|   '(' expr ')'
|   cint
|   "true"
|   "false"
|   re
|   cchar
|   "nil"
|   call
|   '@' expr              %prec '@'
|   "not" expr            %prec UNARY
|   '+' expr              %prec UNARY
|   '-' expr              %prec UNARY
|   expr '+' expr
|   expr '-' expr
|   expr '*' expr
|   expr '/' expr
|   expr "div" expr
|   expr "mod" expr
|   expr "or" expr
|   expr "and" expr
|   expr '=' expr
|   expr "<>" expr
|   expr '<' expr
|   expr "<=" expr
|   expr '>' expr
|   expr ">=" expr
;


call:
    id '(' args ')'
;

args:
    %empty
|   expr exprs
;

exprs:
    %empty
|   exprs ',' expr  
;

%%

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << "\033[91mError: " << m << " at line "
            << l.begin.line << "\033[0m" << std::endl;
  std::exit(1);
}

int main() {
    return 0;
}

     /*
void yyerror(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}
    
int main() {
    int result = yyparse();
    if (result == 0) printf("Success\n");
    return result;
}
     */
