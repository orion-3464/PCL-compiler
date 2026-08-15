#include "ast.hpp"
#include "error.hpp"

#include <iostream>

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

void Block::sem_analyze() {
    for (const auto& s: stmts)
        s->sem_analyze();
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

void Assignment::sem_analyze() {
    lval->sem_analyze();
    e->sem_analyze();

    Lvalue* is_lvalue = dynamic_cast<Lvalue*>(lval.get());
    if(!is_lvalue) {
        ErrorHandler::error(lval->getLoc(), "Assignment: Only l-values can be assigned");
        return;
    }

    const Type* lval_type = lval->getExprType();
    const Type* expr_type = e->getExprType();
    if(!lval_type || !expr_type) return;
    
    if(!lval_type->isCompatible(expr_type)) {
        ErrorHandler::error(lval->getLoc(), "Assignment: Assigned value not compatible");
        return;
    }
}

void Assignment::printNode(std::ostream& out) const {
    out << "Assign( " << *lval << ", " << *e << " )";
}

/* ==================== Conditional ==================== */

void Conditional::sem_analyze() {
    cond->sem_analyze();
    const Type* cond_type = cond->getExprType();
    if (cond_type==nullptr || cond_type->getType()!=PCLtype::Boolean) 
        ErrorHandler::error(cond->getLoc(), "If: Conditions must be of boolean type");

    then_stmt->sem_analyze();
    if (else_stmt) else_stmt->sem_analyze(); 
}

void Conditional::printNode(std::ostream& out) const {
    out << "Conditional( if: "
        << *cond << " -> " << *then_stmt;
    if(else_stmt) out << " : " << *else_stmt;
    out << " )";
}

/* ==================== While ==================== */

void While::sem_analyze() {
    cond->sem_analyze();
    const Type* cond_type = cond->getExprType();
    if (cond_type==nullptr || cond_type->getType()!=PCLtype::Boolean)
        ErrorHandler::error(cond->getLoc(), "While: Conditions must be of boolean type");

    loop->sem_analyze();
}

void While::printNode(std::ostream& out) const {
    out << "While( cond: " << *cond << " do " << *loop << " )"; 
}

/* ==================== Labeling ==================== */

void Labeling::sem_analyze() {
    // TODO:: scopes and symbol table
    code->sem_analyze();
}

void Labeling::printNode(std::ostream& out) const {
    out << "Labeling( label: " << label << " do " << *code << " )"; 
}

/* ==================== Goto ==================== */

void Goto::sem_analyze() {
    // TODO:: scopes and symbol table
}

void Goto::printNode(std::ostream& out) const {
    out << "Goto( label: " << label <<  " )"; 
}

/* ==================== Return ==================== */

void Return::sem_analyze() {
    // TODO:: scopes and symbol table
}

void Return::printNode(std::ostream& out) const {
    out << "Return()"; 
}

/* ==================== New ==================== */

void New::sem_analyze() {
    lvl->sem_analyze();
    const Type* lvl_type = lvl->getExprType();
    const PointerType* lvl_type_ptr = dynamic_cast<const PointerType*>(lvl_type);
    if (!lvl_type_ptr) {
        ErrorHandler::error(lvl->getLoc(), "New: L-value must be of pointer type");
        return;
    }
    const Type* tar_type = lvl_type_ptr->getTargetType();
    
    if(!size) {
        if(!tar_type->isComplete()) 
            ErrorHandler::error(lvl->getLoc(), "New: Pointer should be pointing to a complete type");
    } else {
        size->sem_analyze();
        const Type* size_type = size->getExprType();
        if (size_type->getType() != PCLtype::Integer)
            ErrorHandler::error(lvl->getLoc(), "New: Allocation size must be of type integer");
        // TODO: If value of size can be computed at compile time, check if it is positive
        if (tar_type->getType() != PCLtype::Array || tar_type->isComplete())
            ErrorHandler::error(lvl->getLoc(), "New: Pointer must point to an incomplete array when allocation size is provided");
    }
}

void New::printNode(std::ostream& out) const {
    out << "New( ";
    if(size) out << "number: " << *size << " of ";
    out << *lvl << " )";
}

/* ==================== Dispose ==================== */

void Dispose::sem_analyze() {
    // TODO: check if a new on the symbol an scope was performed
    lvl->sem_analyze();
    const Type* lvl_type = lvl->getExprType();
    const PointerType* lvl_type_ptr = dynamic_cast<const PointerType*>(lvl_type);
    if (!lvl_type_ptr) {
        ErrorHandler::error(lvl->getLoc(), "Dispose: L-value must be of pointer type");
        return;
    }
    const Type* tar_type = lvl_type_ptr->getTargetType();
    if (!brackets) {
        if(!tar_type->isComplete()) 
            ErrorHandler::error(lvl->getLoc(), "Dispose: Pointer should be pointing to a complete type");
    } else {
        if (tar_type->getType() != PCLtype::Array || tar_type->isComplete())
            ErrorHandler::error(lvl->getLoc(), "Dispose: Pointer must point to an incomplete array when allocation size is provided");
    }
}

void Dispose::printNode(std::ostream& out) const {
    out << "Dispose( " << *lvl << " )";
}

/* ==================== Id ==================== */

void Id::sem_analyze() {
    // TODO: symbol table lookup
    return;
}

void Id::printNode(std::ostream& out) const {
    out << "Id( " << name << " )";
}

/* ==================== Result ==================== */

void Result::sem_analyze() {
    // TODO: symbol table lookup
    return;
}

void Result::printNode(std::ostream& out) const  {
    out << "Result()";
}

/* ==================== Str ==================== */

void Str::sem_analyze() {
    // +1 for null termination
    type = std::make_unique<ArrayType>(
        value.length()+1,
        std::make_unique<BasicType>(PCLtype::Char));
}

void Str::printNode(std::ostream& out) const {
    out << "String( " << value << " )";
}

/* ==================== Access ==================== */

void Access::sem_analyze() {
    // TODO: check symbol table if array was previously declared, size etc
    array_base->sem_analyze();
    index->sem_analyze();

    const ArrayType* array_base_ptr = dynamic_cast<const ArrayType*>(array_base->getExprType());
    if (!array_base_ptr) {
        ErrorHandler::error(array_base->getLoc(), "Array access: The symbol indexed must be of array type");
        return;
    }
    
    const Type* index_type = index->getExprType();
    if (index_type->getType() != PCLtype::Integer) {
        ErrorHandler::error(index->getLoc(), "Array access: Index must be of type integer");
        return;
    }

    type = array_base_ptr->getElementType()->cloneType();
}

void Access::printNode(std::ostream& out) const {
    out << "Access( " << *array_base << " [ " << *index << " ] )";
}

/* ==================== Deref ==================== */

void Deref::sem_analyze() {
    // TODO: check symbol table if pointer was previously declared
    pointer->sem_analyze();
    const Type* pointer_type = pointer->getExprType();

    if (pointer_type && pointer_type->getType() == PCLtype::Nil) {
        ErrorHandler::error(pointer->getLoc(), "Dereferencing 'nil' is not allowed");
        return;
    }

    const PointerType* ptr_type = dynamic_cast<const PointerType*>(pointer_type);
    if (!ptr_type) {
        ErrorHandler::error(pointer->getLoc(), "Dereference operator requires a pointer type");
        return;
    }

    type = ptr_type->getTargetType()->cloneType();
}

void Deref::printNode(std::ostream& out) const {
    out << "Deref( " << *pointer << " )";
}

/* ==================== Const ==================== */

void Const::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Integer);
}

void Const::printNode(std::ostream& out) const {
    out << "Const( " << num << " )";
}

/* ==================== Bool ==================== */

void Bool::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Boolean);
}

void Bool::printNode(std::ostream& out) const {
    out << "Bool( " << val << " )";
}

/* ==================== Real ==================== */

void Real::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Real);
}

void Real::printNode(std::ostream& out) const {
    out << "Real( " << val << " )";
}

/* ==================== Char ==================== */

void Char::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Char);
}

void Char::printNode(std::ostream& out) const {
    out << "Char( " << val << " )";
}

/* ==================== Nil ==================== */

void Nil::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Nil);
}

void Nil::printNode(std::ostream& out) const {
    out << "Nil()";
}

/* ==================== Ref ==================== */

void Ref::sem_analyze() {
    left_value->sem_analyze();

    Lvalue* is_lvalue = dynamic_cast<Lvalue*>(left_value.get());
    if(!is_lvalue) {
        ErrorHandler::error(left_value->getLoc(), "Reference: Only l-values can be referenced");
        return;
    }
    
    const Type* lfv_type = left_value->getExprType();
    if(!lfv_type) return;

    type = std::make_unique<PointerType>(lfv_type->cloneType());
}

void Ref::printNode(std::ostream& out) const {
    out << "Ref( " << *left_value << " )";
}

/* ==================== Unop ==================== */

void Unop::sem_analyze() {
    expr->sem_analyze();

    const Type* expr_type = expr->getExprType();
    if(!expr_type) return;

    if (op == UnOp::Plus || op == UnOp::Minus) {
        if(!expr_type->isArithmetic()) { 
            ErrorHandler::error(expr->getLoc(), "Unary Operation Sign (+-): Operand must be of arithmetic type (integer | real)");
            return;
        }
    } else {
        if (expr_type->getType() != PCLtype::Boolean) {
            ErrorHandler::error(expr->getLoc(), "Unary Operation Negation (not): Operand must be of boolean type");
            return;
        }
    }
    type = expr_type->cloneType();
}

void Unop::printNode(std::ostream& out) const {
    out << "Unop( " << to_string(op) << ", " << *expr << " )";
}

/* ==================== Binop ==================== */

void Binop::sem_analyze() { 
    left->sem_analyze();
    right->sem_analyze();

    const Type* left_type = left->getExprType();
    const Type* right_type = right->getExprType();

    if (!left_type || !right_type) return;

    if (op == BinOp::Add || op == BinOp::Sub || op == BinOp::Mul || op == BinOp::Div) {
        if (!(left_type->isArithmetic() && right_type->isArithmetic())) {
            ErrorHandler::error(left->getLoc(), "Binary Operation: Arithmetic operators require operands of arithmetic type");
            return;
        }
        if (op == BinOp::Div) {
            type = std::make_unique<BasicType>(PCLtype::Real);
        } else {
            type = (left_type->getType() == PCLtype::Integer && right_type->getType() == PCLtype::Integer) ?
                   std::make_unique<BasicType>(PCLtype::Integer) :
                   std::make_unique<BasicType>(PCLtype::Real);
        }
    }
    else if (op==BinOp::IntDiv || op==BinOp::Mod) {
        if (!(left_type->getType() == PCLtype::Integer && right_type->getType() == PCLtype::Integer)) {
            ErrorHandler::error(left->getLoc(), "Binary Operation: Integer division and modulo require operands of type integer");
            return;
        }
        type = std::make_unique<BasicType>(PCLtype::Integer);
    }
    else if (op==BinOp::Eq || op==BinOp::Neq) {
        bool both_arithmetic = left_type->isArithmetic() && right_type->isArithmetic();
        bool same_type_not_array = left_type->isSame(right_type) && left_type->getType() != PCLtype::Array;

        if (!both_arithmetic && !same_type_not_array) {
            ErrorHandler::error(left->getLoc(), "Binary Operation: Equality requires operands of same non-array type or both arithmetic");
            return;
        }
        type = std::make_unique<BasicType>(PCLtype::Boolean);
    }
    else if (op==BinOp::Lt || op==BinOp::Gt || op==BinOp::Le || op==BinOp::Ge) {
        if (!(left_type->isArithmetic() && right_type->isArithmetic())) {
            ErrorHandler::error(left->getLoc(), "Binary Operation: Relational operators require operands of arithmetic type");
            return;
        }
        type = std::make_unique<BasicType>(PCLtype::Boolean);
    } else {
        // And, Or
        if(!(left_type->getType()==PCLtype::Boolean && right_type->getType()==PCLtype::Boolean)) {
            ErrorHandler::error(left->getLoc(), "Binary Operation: Conjuction/disjunction require operands of type boolean");
            return;
        }
        type = std::make_unique<BasicType>(PCLtype::Boolean);
    }
}

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

PCLtype BasicType::inferType() const {
    return type;
}

PCLtype BasicType::getType() const {
    return type;
}

std::unique_ptr<Type> BasicType::cloneType() const {
    return std::make_unique<BasicType>(this->type);
}

bool BasicType::isSame(const Type *other) const {
    const BasicType* b = dynamic_cast<const BasicType*>(other);
    return (!b) ? false : (b->inferType() == this->inferType()); 
}

bool BasicType::isCompatible(const Type* other) const {
    if (this->isSame(other)) return true;
    else if (!other->isComposite() && (type == PCLtype::Real && other->inferType() == PCLtype::Integer)) return true;
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

PCLtype ArrayType::inferType() const {
    return element_type->inferType();
}

PCLtype ArrayType::getType() const {
    return type;
}

std::unique_ptr<Type> ArrayType::cloneType() const {
    return std::make_unique<ArrayType>(this->size, this->element_type->cloneType());
}

bool ArrayType::isSame(const Type *other) const {
    const ArrayType* a = dynamic_cast<const ArrayType*>(other); 
    return (a && (size == a->getLength())) ? element_type->isSame(a->getElementType()) : false; 
}

bool ArrayType::isCompatible(const Type* other) const {
    if (this->isSame(other)) return true;

    const ArrayType* other_array = dynamic_cast<const ArrayType*>(other);
    if (other_array)
        return !this->isComplete() && other_array->isComplete() && 
               this->getElementType()->isSame(other_array->getElementType());
    
    return false;
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

PCLtype PointerType::inferType() const {
    return target->inferType();
}

PCLtype PointerType::getType() const {
    return type;
}

std::unique_ptr<Type> PointerType::cloneType() const {
    return std::make_unique<PointerType>(this->target->cloneType());
}

bool PointerType::isSame(const Type *other) const {
    const PointerType* p = dynamic_cast<const PointerType*>(other); 
    return (p) ? target->isSame(p->getTargetType()) : false; 
}

bool PointerType::isCompatible(const Type* other) const {
    if (other->getType() == PCLtype::Nil) return true;
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
