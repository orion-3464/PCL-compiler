#include "ast/symbol.hpp"
#include <memory>

SymbolTable::SymbolTable() {
    current_depth = 0;
    enterScope();

    static std::unique_ptr<Type> type_int = std::make_unique<BasicType>(PCLtype::Integer);
    static std::unique_ptr<Type> type_real = std::make_unique<BasicType>(PCLtype::Real);
    static std::unique_ptr<Type> type_bool = std::make_unique<BasicType>(PCLtype::Boolean);
    static std::unique_ptr<Type> type_char = std::make_unique<BasicType>(PCLtype::Char);
    static std::unique_ptr<Type> type_char_array = std::make_unique<ArrayType>(-1, std::make_unique<BasicType>(PCLtype::Char));

    const Type* t_int = type_int.get();
    const Type* t_real = type_real.get();
    const Type* t_bool = type_bool.get();
    const Type* t_char = type_char.get();
    const Type* t_char_array = type_char_array.get();
    
    std::vector<Parameter> int_parameter = {Parameter(false, t_int)};
    std::vector<Parameter> bool_parameter = {Parameter(false, t_bool)};
    std::vector<Parameter> real_parameter = {Parameter(false, t_real)};
    std::vector<Parameter> char_parameter = {Parameter(false, t_char)};
    std::vector<Parameter> char_array_parameter = {Parameter(true, t_char_array)};
    std::vector<Parameter> char_ptr_parameter = {Parameter(false, t_int), Parameter(true, t_char_array)};
    
    insert("writeInteger", RoutineName(false, nullptr, int_parameter));
    insert("writeBoolean", RoutineName(false, nullptr, bool_parameter));
    insert("writeChar", RoutineName(false, nullptr, char_parameter));
    insert("writeReal", RoutineName(false, nullptr, real_parameter));
    insert("writeString", RoutineName(false, nullptr, char_array_parameter));

    insert("readInteger", RoutineName(false, t_int, {}));
    insert("readBoolean", RoutineName(false, t_bool, {}));
    insert("readChar", RoutineName(false, t_char, {}));
    insert("readReal", RoutineName(false, t_real, {}));
    insert("readString", RoutineName(false, nullptr, char_ptr_parameter));

    insert("abs", RoutineName(false, t_int, int_parameter));
    insert("fabs", RoutineName(false, t_real, real_parameter));

    insert("sqrt", RoutineName(false, t_real, real_parameter));
    insert("sin", RoutineName(false, t_real, real_parameter));
    insert("cos", RoutineName(false, t_real, real_parameter));
    insert("tan", RoutineName(false, t_real, real_parameter));
    insert("arctan", RoutineName(false, t_real, real_parameter));
    insert("exp", RoutineName(false, t_real, real_parameter));
    insert("ln", RoutineName(false, t_real, real_parameter));
    insert("pi", RoutineName(false, t_real, {}));

    insert("trunc", RoutineName(false, t_int, real_parameter));
    insert("round", RoutineName(false, t_int, real_parameter));
    insert("ord", RoutineName(false, t_int, char_parameter));
    insert("chr", RoutineName(false, t_char, int_parameter));
}

void SymbolTable::enterScope() {
    scopes.push_back({});
    ++current_depth;
}

void SymbolTable::exitScope() {
    for(const auto& name : scopes.back()) {
        symbol_table[name].pop_back();
        if (symbol_table[name].empty()) symbol_table.erase(name);
    }
    
    scopes.pop_back();
    --current_depth;
}

bool SymbolTable::insert(const std::string& s, const std::variant<Variable, Parameter, RoutineName, Label>& v) {
    auto it = symbol_table.find(s);
    if (it!=symbol_table.end() && !it->second.empty())
        if (it->second.back().depth==current_depth)
            return false;
    
    symbol_table[s].push_back(SymTabEntry(s, current_depth, v));
    scopes.back().push_back(s);
    return true;
}

SymTabEntry* SymbolTable::lookup(const std::string& s) {
    auto it = symbol_table.find(s);
    return (it != symbol_table.end() && !it->second.empty()) ?
        &(it->second.back()) : nullptr;
}

unsigned int SymbolTable::getCurrentDepth() const {
    return current_depth;
}
