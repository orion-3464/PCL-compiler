#include "error.hpp"

int ErrorHandler::error_count = 0;

void ErrorHandler::error(const yy::location &loc, const std::string &msg) {
    error_count++;

    std::cerr << "\033[91mError\033[0m at line " << loc.begin.line 
              << ", column " << loc.begin.column << ": " 
              << msg << std::endl;
}

void ErrorHandler::warning(const yy::location &loc, const std::string &msg) {
    std::cerr << "\033[93mWarning\033[0m at line " << loc.begin.line 
              << ", column " << loc.begin.column << ": " 
              << msg << std::endl;
}

int ErrorHandler::getErrorCount() {
    return error_count;
}
