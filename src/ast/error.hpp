#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include <iostream>
#include <string>
#include "location.hpp"

class ErrorHandler {
public:
    static void error(const yy::location& loc, const std::string& msg);
    static void warning(const yy::location& loc, const std::string& msg);

    static int getErrorCount();

private:
    static int error_count;
};

#endif // __ERROR_HPP__
