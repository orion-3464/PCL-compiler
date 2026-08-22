#ifndef __SYMBOL_HPP__
#define __SYMBOL_HPP__

#include <unordered_map>
#include <vector>
#include <variant>
#include <string>

#include "ast.hpp"
#include "error.hpp"

struct Variable {
    const Type* type;

    Variable() = default;
    Variable(const Type* t) : type(t) {}
};

struct Parameter {
    bool is_reference;
    const Type* type;

    Parameter() = default;
    Parameter(bool r, const Type* t) : is_reference(r), type(t) {}
};

struct RoutineName {
    bool is_forward;
    const Type* retType;
    std::vector<Parameter> parameters;

    RoutineName() = default;
    RoutineName(bool f, const Type* t, const std::vector<Parameter>& p) :
        is_forward(f), retType(t), parameters(p) {}
};

struct Label {
    bool is_defined;

    Label() = default;
    Label(bool d) : is_defined(d) {}
};

struct SymTabEntry {
    std::string name;
    unsigned int depth;
    std::variant<Variable, Parameter, RoutineName, Label> sym_info;

    SymTabEntry() = default;
    SymTabEntry(const std::string& n, unsigned int d, const std::variant<Variable, Parameter, RoutineName, Label>& v) :
        name(n), depth(d), sym_info(v) {}
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool insert(const std::string &s, const std::variant<Variable, Parameter, RoutineName, Label>& v);
    SymTabEntry* lookup(const std::string& s);
    unsigned int getCurrentDepth() const;
private:
    std::unordered_map<std::string, std::vector<SymTabEntry>> symbol_table;
    std::vector<std::vector<std::string>> scopes;
    unsigned int current_depth;
};

#endif // __SYMBOL_HPP__
