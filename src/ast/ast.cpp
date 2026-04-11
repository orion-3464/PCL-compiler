#include "ast.hpp"
#include <memory>
#include <ostream>

std::string to_string(BinOp op) {
    switch(op) {
        case BinOp::Add:    return "+";
        case BinOp::Sub:    return "-";
        case BinOp::Mul:    return "*";
        case BinOp::Div:    return "/";
        case BinOp::IntDiv: return "div";
        case BinOp::Mod:    return "mod";
        case BinOp::And:    return "and";
        case BinOp::Or:     return "or";
        case BinOp::Eq:     return "=";
        case BinOp::Neq:    return "<>";
        case BinOp::Lt:     return "<";
        case BinOp::Le:     return "<=";
        case BinOp::Gt:     return ">";
        case BinOp::Ge:     return ">=";
        default:            return "unknown";
    }
}

std::string to_string(UnOp op) {
    switch(op) {
        case UnOp::Not: return "not";
        case UnOp::Plus: return "+";
        case UnOp::Minus: return "-";
        default: return "unknown";
    }
}

/* ==================== Block ==================== */

void Block::append(std::unique_ptr<Stmt> s)  {
    stmts.push_back(std::move(s));
}

void Block::printNode(std::ostream& out) const {
    out << "Block( ";
    bool first = true;
    for (const auto &s : stmts) {
        if(!first) out << ", ";
        out << *s;
        first = false;
    }
    out << " )";
}

/* ==================== Empty  ==================== */

void Empty::printNode(std::ostream& out) const  {
    out << "Empty()";
}

/* ==================== Assignment ==================== */

void Assignment::printNode(std::ostream& out) const {
    out << "Assign( " << *lval << ", " << *e << " )";
}

/* ==================== Conditional ==================== */

void Conditional::printNode(std::ostream& out) const {
    out << "Conditional( if: "
        << *cond << " -> " << *then_stmt;
    if(else_stmt) out << " : " << *else_stmt;
    out << " )";
}

/* ==================== While ==================== */

void While::printNode(std::ostream& out) const {
    out << "While( cond: " << *cond << " do " << *loop << " )"; 
}

/* ==================== Labeling ==================== */

void Labeling::printNode(std::ostream& out) const {
    out << "Labeling( label: " << label << " do " << *code << " )"; 
}

/* ==================== Goto ==================== */

void Goto::printNode(std::ostream& out) const {
    out << "Goto( label: " << label <<  " )"; 
}

/* ==================== Return ==================== */

void Return::printNode(std::ostream& out) const {
    out << "Return()"; 
}

/* ==================== New ==================== */

void New::printNode(std::ostream& out) const {
    out << "New( ";
    if(size) out << "number: " << *size << " of ";
    out << *lvl << " )";
}

/* ==================== Dispose ==================== */

void Dispose::printNode(std::ostream& out) const {
    out << "Dispose( " << *lvl << " )";
}

/* ==================== Id ==================== */

void Id::printNode(std::ostream& out) const {
    out << "Id( " << name << " )";
}

/* ==================== Result ==================== */

void Result::printNode(std::ostream& out) const  {
    out << "Result()";
}

/* ==================== Str ==================== */

void Str::printNode(std::ostream& out) const {
    out << "String( " << value << " )";
}

/* ==================== Access ==================== */

void Access::printNode(std::ostream& out) const {
    out << "Access( " << *array_base << " [ " << *index << " ] )";
}

/* ==================== Deref ==================== */

void Deref::printNode(std::ostream& out) const {
    out << "Deref( " << *pointer << " )";
}

/* ==================== Const ==================== */

void Const::printNode(std::ostream& out) const {
    out << "Const( " << num << " )";
}

/* ==================== Bool ==================== */

void Bool::printNode(std::ostream& out) const {
    out << "Bool( " << val << " )";
}

/* ==================== Real ==================== */

void Real::printNode(std::ostream& out) const {
    out << "Real( " << val << " )";
}

/* ==================== Char ==================== */

void Char::printNode(std::ostream& out) const {
    out << "Char( " << val << " )";
}

/* ==================== Nil ==================== */

void Nil::printNode(std::ostream& out) const {
    out << "Nil()";
}

/* ==================== Ref ==================== */

void Ref::printNode(std::ostream& out) const {
    out << "Ref( " << *left_value << " )";
}

/* ==================== Unop ==================== */

void Unop::printNode(std::ostream& out) const {
    out << "Unop( " << to_string(op) << ", " << *expr << " )";
}

/* ==================== Binop ==================== */

void Binop::printNode(std::ostream& out) const {
    out << "Binop( " << to_string(op) << " | "
        << *left << " | " << *right << " )";
}

/* ==================== CallProc ==================== */

void CallProc::printNode(std::ostream& out) const {
    out << "CallProc( name : " << name << " args: ";
    bool first = true;
    for (const auto &a : args) {
        if(!first) out << ", ";
        out << *a;
        first = false;
    }
    out << " )";
}

/* ==================== CallFunc ==================== */

void CallFunc::printNode(std::ostream& out) const {
    out << "CallFunc( name : " << name << " args: ";
    bool first = true;
    for (const auto &a : args) {
        if(!first) out << ", ";
        out << *a;
        first = false;
    }
    out << " )";
}

/* ==================== Basic Type ==================== */

int BasicType::getSize() const {
    if(name == "integer") return 4;
    else if(name == "real") return 8;
    else if (name == "boolean" || name == "char") return 1;
    else return 0;
}

bool BasicType::isComplete() const { 
    return true; 
}

bool BasicType::isArithmetic() const {
    return (name == "integer" || name == "real");
}

void BasicType::printNode(std::ostream& out) const {
    out << "BasicType(" << name << ")";
}

/* ==================== Array Type ==================== */

int ArrayType::getSize() const {
    if (!isComplete()) return 0;
    return size * element_type->getSize();
}

bool ArrayType::isComplete() const { 
    return size>0; 
}

bool ArrayType::isArithmetic() const {
    return false;
}

void ArrayType::printNode(std::ostream& out) const {
    out << "ArrayType( element type: " << *element_type
        << " size: " << size << " )";
}

/* ==================== Pointer Type ==================== */

int PointerType::getSize() const {
    return 8;
}

bool PointerType::isComplete() const { 
    return true; 
}

bool PointerType::isArithmetic() const {
    return false;
}

void PointerType::printNode(std::ostream& out) const {
    out << "PointerType( element type: " << *target << " )";
}


/* ==================== Body ==================== */

void Body::printNode(std::ostream& out) const  {
    out << "Body(\n";
    for (const auto& local : locals) {
        out << "  " << *local << "\n";
    }
    out << "  " << *block << "\n)";
}

/* ==================== Var ==================== */

void Var::printNode(std::ostream& out) const {
    out << "Var( names: ";
    for(const auto& i : ids)
        out << i << ", ";
    out << "type: " << *type << " )";
}

/* ==================== LabelDef ==================== */

void LabelDef::printNode(std::ostream& out) const {
    out << "LabelDef( names: ";
    bool first = true;
    for(const auto& i : ids) {
        if(!first) out << ", ";
        out << i;
        first = false;
    }
    out << " )";
}

/* ==================== Formal ==================== */

void Formal::printNode(std::ostream& out) const  {
    out << "Formal( ";
    if (is_reference) out << "var ";
    for (const auto& id : ids) out << id << " ";
    out << ": " << *type << " )";
}

/* ==================== RoutineDef ==================== */

void RoutineDef::printNode(std::ostream& out) const {
    (return_type) ? out << "FunctionDef( " << name << " (" 
        : out << "ProcedureDef( " << name << " (";
 
    bool first = true;
    for (const auto& f : formals) {
        if (!first) out << "; ";
        out << *f;
        first = false;
    }
    out << ") ";
    
    if (return_type) out << ": " << *return_type << " ";

    (body) ? out << "\n" << *body << "\n)"
        : out << " [forward] )"; 
}

/* ==================== Program ==================== */

void Program::printNode(std::ostream& out) const {
    out << "Program( " << name << "\n" << *body << "\n)";
}
