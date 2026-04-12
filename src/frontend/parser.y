%require "3.8"
%language "c++"
%locations
%verbose
%define api.location.file "location.hpp"

%code top {
    #include <iostream>
    #include <string>
    #include <memory>
    #include <vector>
    #include <utility>
}

%code requires {
    #include "ast.hpp"
}

%code {
    #include "lexer.hpp"
    YY_DECL;

    static std::unique_ptr<Program> programAST;
}

%define api.value.type variant
%define api.token.constructor
%define api.value.automove
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
                        
%token<std::string> id    
%token<unsigned long> cint  
%token<double> re    
%token<char> cchar 
%token<std::string> str

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

%nterm<std::unique_ptr<Program>> program
%nterm<std::unique_ptr<Body>> body
%nterm<std::unique_ptr<Block>> block

%nterm<std::vector<std::unique_ptr<Local>>> locals local definitions
/*%nterm<std::vector<std::unique_ptr<Var>>> definitions*/
%nterm<std::unique_ptr<Local>> definition

%nterm<HeaderInfo> header
%nterm<std::vector<std::unique_ptr<Formal>>> parameters formals
%nterm<std::unique_ptr<Formal>> formal

%nterm<bool> opvar brackop
%nterm<std::vector<std::string>> ids
%nterm<int> size

%nterm<std::unique_ptr<Type>> type

%nterm<std::unique_ptr<Stmt>> stmt elseop
%nterm<std::vector<std::unique_ptr<Stmt>>> stmts 

%nterm<std::unique_ptr<Expr>> expr exprop
%nterm<std::vector<std::unique_ptr<Expr>>> exprs args

%nterm <std::pair<std::string, std::vector<std::unique_ptr<Expr>>>> call

%expect 1 /* dangling else */
   
%%

program:
    "program" id ';' body '.' { programAST = std::make_unique<Program>($2, $4, @$); }
;

body:
    locals block { $$ = std::make_unique($1, $2, @$); }
;

locals:
    %empty       { $$ = std::vector<std::unique_ptr<Local>>(@$); }
|   locals local {
                     for(auto& l : $2)
                         $1.push_back(l); $$ = $1;
                 }
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
    id ids ':' type ';' {
                            
                        }
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
    "integer" { $$ = std::make_unique<BasicType>("integer", @$); }
|   "real" { $$ = std::make_unique<BasicType>("real", @$); }
|   "boolean" { $$ = std::make_unique<BasicType>("boolean", @$); }
|   "char" { $$ = std::make_unique<BasicType>("char", @$); }
|   "array" size "of" type { $$ = std::make_unique<ArrayType>($2, $4, @$); }
|   '^' type { $$ = std::make_unique<PointerType>($2, @$); }
;

size:
    %empty { $$ = -1; }
|   '[' cint ']' { $$ = $2; }
;

block:
    "begin" stmt stmts "end" {  }
;

stmts:
    %empty
|   stmts ';' stmt
;

stmt:
    %empty { std::make_unique<Empty>(@$); }
|   expr ":=" expr { std::make_unique<Assignment>($1, $3, @$); }
|   block { $$ = $1; }
|   call { std::make_unique<CallProc>($1.first, $1.second, @$); }
|   "if" expr "then" stmt elseop { std::make_unique<Conditional>($2, $4, $5, @$); }
|   "while" expr "do" stmt { std::make_unique<While>($2, $4, @$); }
|   id ':' stmt { std::make_unique<Labeling>($1, $3, @$); }
|   "goto" id { std::make_unique<Goto>($2, @$); }
|   "return" { std::make_unique<Return>(@$); }
|   "new" exprop expr { std::make_unique<New>($2, $3, @$); }
|   "dispose" brackop expr { std::make_unique<Dispose>($2, $3, @$); }
;

elseop:
    %empty { $$ = nullptr; }
|   "else" stmt { $$ = $2; }
;

exprop:
    %empty { $$ = nullptr; }
|   '[' expr ']' { $$ = $2; }
;

brackop:
    %empty { $$ = false; }
|   '['']' { $$ = true; }
;

expr:
    id  { $$ = std::make_unique<Id>($1, @$); }
|   "result" { $$ = std::make_unique<Result>(@$); }
|   str { $$ = std::make_unique<Str>($1, @$); }
|   expr '[' expr ']'
|   expr '^'              %prec '^'
|   '(' expr ')' { $$ = std::make_unique<Expr>($2, @$); }
|   cint { $$ = std::make_unique<Const>($1, @$); }
|   "true" { $$ = std::make_unique<Bool>(true, @$); }
|   "false" { $$ = std::make_unique<Bool>(false, @$); }
|   re { $$ = std::make_unique<Real>($1, @$); }
|   cchar { $$ = std::make_unique<Char>($1, @$); }
|   "nil" { $$ = std::make_unique<Nil>(@$); }
|   call
|   '@' expr              %prec '@'
|   "not" expr            %prec UNARY
|   '+' expr              %prec UNARY
|   '-' expr              %prec UNARY
|   expr '+' expr { $$ = std::make_unique<Binop>($1, BinOp::Add, $3, @$); }
|   expr '-' expr { $$ = std::make_unique<Binop>($1, BinOp::Sub, $3, @$); }
|   expr '*' expr { $$ = std::make_unique<Binop>($1, BinOp::Mul, $3, @$); }
|   expr '/' expr { $$ = std::make_unique<Binop>($1, BinOp::Div, $3, @$); }
|   expr "div" expr { $$ = std::make_unique<Binop>($1, BinOp::IntDiv, $3, @$); }
|   expr "mod" expr { $$ = std::make_unique<Binop>($1, BinOp::Mod, $3, @$); }
|   expr "or" expr { $$ = std::make_unique<Binop>($1, BinOp::Or, $3, @$); }
|   expr "and" expr { $$ = std::make_unique<Binop>($1, BinOp::And, $3, @$); }  
|   expr '=' expr { $$ = std::make_unique<Binop>($1, BinOp::Eq, $3, @$); }
|   expr "<>" expr { $$ = std::make_unique<Binop>($1, BinOp::Neq, $3, @$); }
|   expr '<' expr { $$ = std::make_unique<Binop>($1, BinOp::Lt, $3, @$); }
|   expr "<=" expr { $$ = std::make_unique<Binop>($1, BinOp::Le, $3, @$); }
|   expr '>' expr { $$ = std::make_unique<Binop>($1, BinOp::Gt, $3, @$); }
|   expr ">=" expr { $$ = std::make_unique<Binop>($1, BinOp::Ge, $3, @$); }
;

call:
    id '(' args ')' { $$ = std::make_pair($1, ); }
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
