%code top {
    #include <cstdio>
    #include <cstdlib>
}

%code requires {
    #include "lexer.hpp"
}

%code {

}

%locations

%token T_and   "and"
%token T_array "array"
%token T_begin "begin"
%token T_bool  "boolean"
%token T_char  "char"
%token T_disp  "dispose"
%token T_div   "div"
%token T_do    "do"
%token T_else  "else"
%token T_end   "end"
%token T_false "false"
%token T_forw  "forward"
%token T_func  "function"
%token T_goto  "goto"
%token T_if    "if"
%token T_int   "integer"
%token T_label "label"
%token T_mod   "mod"
%token T_new   "new"
%token T_nil   "nil"
%token T_not   "not"
%token T_of    "of"
%token T_or    "or"
%token T_proc  "procedure"
%token T_prog  "program"
%token T_real  "real"
%token T_res   "result"
%token T_ret   "return"
%token T_then  "then"
%token T_true  "true"
%token T_var   "var"
%token T_while "while"
                        
%token T_id    
%token T_cint  
%token T_re    
%token T_cchar 
%token T_str

%token T_neq  "<>" 
%token T_geq  ">=" 
%token T_leq  "<=" 
%token T_asgn ":="

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
    "program" T_id ';' body '.'
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
|   "label" T_id ids ';'
|   header ';' body ';'
|   "forward" header ';'
;

definitions:
    definition
|   definitions definition
;

definition:
    T_id ids ':' type ';'
;

ids:
    %empty
|   ids ',' T_id
;

header:
    "procedure" T_id '(' parameters ')' /* parameters are optional */
|   "function" T_id '(' parameters ')' ':' type
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
    opvar T_id ids ':' type
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
|   '[' T_cint ']'
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
|   T_id ':' stmt
|   "goto" T_id
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
    T_id
|   "result"
|   T_str
|   expr '[' expr ']'
|   expr '^'              %prec '^'
|   '(' expr ')'
|   T_cint
|   "true"
|   "false"
|   T_re
|   T_cchar
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
    T_id '(' args ')'
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

void yyerror(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}
    
int main() {
    int result = yyparse();
    if (result == 0) printf("Success\n");
    return result;
}
