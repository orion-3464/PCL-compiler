#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "parser.hpp" 
#include "ast.hpp"
#include "symbol.hpp"

extern FILE *yyin;
extern void yyrestart(FILE *inp);

extern std::unique_ptr<Program> programAST;
SymbolTable symtab;

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = std::fopen(argv[1], "r");
        if (!file) {
            std::cerr << "\033[91mError: Could not open file "
                      << argv[1] << "\033[0m" << std::endl;
            return 1;
        }
        yyin = file;
        yyrestart(yyin); 
    }
    else {
        yyin = stdin;
    }
    
    yy::parser p;
    
    int result = p.parse();
    
    if (result == 0) {
        std::cout << "\033[31;42mSuccess: File completely parsed!\033[0m" << std::endl;
        if (programAST){
            std::cout << *programAST << std::endl;
            programAST->sem_analyze();
            
        }
        if (ErrorHandler::getErrorCount() == 0) {
            std::cout << "Semantic analysis completed successfully!" << std::endl;
        }
        else std::cout << "Warning: Parsing succeeded, but AST is null." << std::endl;
    }
    
    return result;
}
