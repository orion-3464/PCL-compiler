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

/* ==================== Expr ==================== */


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
    if(type == PCLtype::Integer) return 4;
    else if(type == PCLtype::Real) return 8;
    else if (type == PCLtype::Boolean || type == PCLtype::Char) return 1;
    else return 0;
}

bool BasicType::isComposite() const {
    return false;
}

bool BasicType::isComplete() const { 
    return true; 
}

bool BasicType::isArithmetic() const {
    return (type == PCLtype::Integer || type == PCLtype::Real);
}

PCLtype BasicType::infer_type() const {
    return type;
}

bool BasicType::isSame(const Type *other) const {
    const BasicType* b = dynamic_cast<const BasicType*>(other);
    return (!b) ? false : (b->infer_type() == this->infer_type()); 
}

bool BasicType::isCompatible(const Type* other) const {
    if (this->isSame(other)) return true;
    else if (!other->isComposite() && (type == PCLtype::Real && other->infer_type() == PCLtype::Integer)) return true;
    else return false;
}

void BasicType::printNode(std::ostream& out) const {
    switch(type) {
    case PCLtype::Integer:
        out << "BasicType( integer )";
        break;
    case PCLtype::Real:
        out << "BasicType( real )";
        break;
    case PCLtype::Boolean:
        out << "BasicType( boolean )";
        break;
    case PCLtype::Char:
        out << "BasicType( char )";
        break;
    default:
        out << "Not a Basic Type";
        break;
    }
}

/* ==================== Array Type ==================== */

const Type* ArrayType::getElementType() const {
    const Type* t = element_type.get();
    return t;
}

int ArrayType::getLength() const {
    return size;
}

int ArrayType::getSize() const {
    if (!isComplete()) return 0;
    return size * element_type->getSize();
}

bool ArrayType::isComposite() const {
    return true;
}

bool ArrayType::isComplete() const { 
    return size>0; 
}

bool ArrayType::isArithmetic() const {
    return false;
}

PCLtype ArrayType::infer_type() const {
    return element_type->infer_type();
}

bool ArrayType::isSame(const Type *other) const {
    const ArrayType* a = dynamic_cast<const ArrayType*>(other); 
    return (a && (size == a->getLength())) ? element_type->isSame(a->getElementType()) : false; 
}

bool ArrayType::isCompatible(const Type* other) const {
    return this->isComplete() && this->isSame(other);
    // TODO: See if conversion is done by the compiler in a for loop
    // else if (other->isComposite() && (element_type->infer_type() == PCLtype::Real && other->infer_type() == PCLtype::Integer)) return true;
}

void ArrayType::printNode(std::ostream& out) const {
    out << "ArrayType( element type: " << *element_type
        << " size: " << size << " )";
}

/* ==================== Pointer Type ==================== */

const Type* PointerType::getTargetType() const {
    const Type* t = target.get();
    return t;
}

int PointerType::getSize() const {
    return 8;
}

bool PointerType::isComposite() const {
    return true;
}

bool PointerType::isComplete() const { 
    return true; 
}

bool PointerType::isArithmetic() const {
    return false;
}

PCLtype PointerType::infer_type() const {
    return target->infer_type();
}

bool PointerType::isSame(const Type *other) const {
    const PointerType* p = dynamic_cast<const PointerType*>(other); 
    return (p) ? target->isSame(p->getTargetType()) : false; 
}

bool PointerType::isCompatible(const Type* other) const {
    const PointerType* pother = dynamic_cast<const PointerType*>(other);
    if (!pother) return false;
    if (this->isSame(other)) return true;

    const ArrayType* thisArrayTarget = dynamic_cast<const ArrayType*>(target.get());
    const ArrayType* otherArrayTarget = dynamic_cast<const ArrayType*>(pother->getTargetType());

    if ((thisArrayTarget && otherArrayTarget) &&
        (!thisArrayTarget->isComplete() && otherArrayTarget->isComplete()) &&
        (thisArrayTarget->getElementType()->isSame(otherArrayTarget->getElementType())))
            return true;
    
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

/* ==================== Header ==================== */

const std::string& Header::getName() const {
    return name;
}

const std::vector<std::unique_ptr<Formal>>& Header::getFormals() const {
    return formals;
}

const Type* Header::getRetType() const {
    return return_type.get();
}

/* ==================== RoutineDef ==================== */

void RoutineDef::printNode(std::ostream& out) const {
    const std::string& name = header->getName();
    const std::vector<std::unique_ptr<Formal>>& formals = header->getFormals();
    const Type* return_type = header->getRetType();
    
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
