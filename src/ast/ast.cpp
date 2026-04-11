#include "ast.hpp"
#include <memory>
#include <ostream>

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
    out << "Unop( " << op << ", " << *expr << " )";
}

/* ==================== Binop ==================== */

void Binop::printNode(std::ostream& out) const {
    out << "Binop( " << op << " | "
        << *left << " | " << *right
        <<" )";
}

/* ==================== CallProc ==================== */

void CallProc::printNode(std::ostream& out) const {
    out << "CallProc( name: " << name
        << " args: ";
    for(const auto& a : args) out << *a;
    out << " )";
}

/* ==================== CallFunc ==================== */

void CallFunc::printNode(std::ostream& out) const {
    out << "CallFunc( name: " << name
        << " args: ";
    for(const auto& a : args) out << *a;
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
