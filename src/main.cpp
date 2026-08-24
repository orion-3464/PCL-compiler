#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "parser.hpp" 
#include "ast/ast.hpp"
#include "ast/symbol.hpp"

extern FILE *yyin;
extern void yyrestart(FILE *inp);

extern std::unique_ptr<Program> programAST;
SymbolTable symtab;

int main(int argc, char* argv[]) {
    int opt;
    CompilerOptions co;
        
    while ((opt=getopt(argc, argv, "o:Ofi")) != -1) {
        switch (opt) {
        case 'O':
            co.optimize = true;
            break;
        case 'f':
            co.final_code_stdout = true;
            break;
        case 'i':
            co.imm_code_stdout = true;
            break;
        case 'o':
            co.ex_name = optarg;
            break;
        default:
            std::cerr << "Usage: ./pclc [-Ofi] [-o <exec_name>]"
                      << std::endl;
            std::exit(1);
        }
    }

    if (co.imm_code_stdout || co.final_code_stdout) {
        yyin = stdin;
    } 
    else {
        if (optind >= argc) {
            std::cerr << "\033[91mError: No input file given\033[0m" << std::endl;
            std::cerr << "Usage: ./pclc [-Ofi] [-o <exec_name>] <filename>" << std::endl;
            std::exit(1);
        }
        
        FILE *file = std::fopen(argv[optind], "r");
        if (!file) {
            std::cerr << "\033[91mError: Could not open file "
                      << argv[optind] << "\033[0m" << std::endl;
            std::exit(1);
        }
        yyin = file;
        co.filename = argv[optind];
    }
    
    yyrestart(yyin);    
    yy::parser p;
    
    int result = p.parse();

    if (result != 0) {
        return result; 
    }
    
    if (programAST) {
        // std::cout << *programAST << std::endl;
        programAST->sem_analyze();
    } else {
        std::cerr << "Warning: Parsing succeeded, but AST is null." << std::endl;
        return 1;
    }
    
    if (ErrorHandler::getErrorCount() == 0) {
        //std::cout << "Semantic analysis completed successfully!" << std::endl;
        programAST->compile(co);
        return 0;
    } else {
        std::cerr << "Semantic analysis failed with " 
                  << ErrorHandler::getErrorCount() << " errors." << std::endl;
        return 1;
    }
}
