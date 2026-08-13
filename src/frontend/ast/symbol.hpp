#ifndef __SYMBOL_HPP__
#define __SYMBOL_HPP__

#include <map>
#include <vector>

#include "error.hpp"

enum class PCLtype {
    Integer,
    Boolean,
    Char,
    Real,
    Array,
    Pointer,
    Nil
};

enum class PCLsymbol {
    Variable,
    ParameterVal,
    ParameterRef,
    Function,
    Procedure,
    Label    
};


#endif // __SYMBOL_HPP__
