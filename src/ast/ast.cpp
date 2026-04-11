#include "ast.hpp"
#include <memory>

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
