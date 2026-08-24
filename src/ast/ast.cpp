#include "ast/ast.hpp"
#include "ast/error.hpp"
#include "ast/symbol.hpp"

#include "backend/codegen.hpp" 

#include <iostream>
#include <sstream>
#include <filesystem>

extern SymbolTable symtab;
static std::vector<const Type*> routine_returns_stack;

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

const Type* Expr::getExprType() const {
    return type.get();
}

std::optional<int> Expr::getConstValue() const {
    return std::nullopt;
}

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

Value* Block::codegen() const {
    for (const auto& s: stmts)
         s->codegen();
    return nullptr;
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
    SymTabEntry* label_entry = symtab.lookup(label);
    if(!label_entry) {
        ErrorHandler::error(getLoc(), "Label not declared");
        return;
    }
    Label* label_info = std::get_if<Label>(&label_entry->sym_info);
    if(!label_info) {
        ErrorHandler::error(getLoc(), "Symbol declared but not as label");
        return;
    }
    if (label_info->is_defined) {
        ErrorHandler::error(getLoc(), "Label '" + label + "' is already defined");
        return;
    }

    label_info->is_defined=true;
    code->sem_analyze();
}

void Labeling::printNode(std::ostream& out) const {
    out << "Labeling( label: " << label << " do " << *code << " )"; 
}

/* ==================== Goto ==================== */

void Goto::sem_analyze() {
    SymTabEntry* label_entry = symtab.lookup(label);
    if(!label_entry) {
        ErrorHandler::error(getLoc(), "Target label not declared");
        return;
    }
    Label* label_info = std::get_if<Label>(&label_entry->sym_info);
    if(!label_info) {
        ErrorHandler::error(getLoc(), "Symbol used in \"goto\" statement is not as label");
        return;
    }
    if (label_entry->depth != symtab.getCurrentDepth()) {
        ErrorHandler::error(getLoc(), "Illegal cross-scope jump: label '" + label + "' is not declared in the current scope");
    }
}

void Goto::printNode(std::ostream& out) const {
    out << "Goto( label: " << label <<  " )"; 
}

/* ==================== Return ==================== */

void Return::printNode(std::ostream& out) const {
    out << "Return()"; 
}

/* ==================== New ==================== */

void New::sem_analyze() {
    lvl->sem_analyze();
    
    Lvalue* is_lvalue = dynamic_cast<Lvalue*>(lvl.get());
    if(!is_lvalue) {
        ErrorHandler::error(lvl->getLoc(), "New: Argument must be an l-value");
        return;
    }
    
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
        if (tar_type->getType() != PCLtype::Array || tar_type->isComplete())
            ErrorHandler::error(lvl->getLoc(), "New: Pointer must point to an incomplete array when allocation size is provided");
        
        auto const_size = size->getConstValue();
        if (const_size.has_value()) {
            if (const_size.value() <= 0) {
                ErrorHandler::error(size->getLoc(), "New: Allocation size must be a positive integer");
            }
        }
    }
}

void New::printNode(std::ostream& out) const {
    out << "New( ";
    if(size) out << "number: " << *size << " of ";
    out << *lvl << " )";
}

/* ==================== Dispose ==================== */

void Dispose::sem_analyze() {
    lvl->sem_analyze();

    Lvalue* is_lvalue = dynamic_cast<Lvalue*>(lvl.get());
    if(!is_lvalue) {
        ErrorHandler::error(lvl->getLoc(), "Dispose: Argument must be an l-value");
        return;
    }
    
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
    SymTabEntry* id_entry = symtab.lookup(name);
    if (!id_entry) {
        ErrorHandler::error(getLoc(), "Name " + name + " not declared");
        return;
    }
    Variable* var_info = std::get_if<Variable>(&id_entry->sym_info);
    Parameter* par_info = std::get_if<Parameter>(&id_entry->sym_info);
    if (var_info) {
        type = var_info->type->cloneType();
    } 
    else if (par_info) {
        type = par_info->type->cloneType();
    } else {
        ErrorHandler::error(getLoc(), "Identifier '" + name + "' is not a variable or parameter");
    }
}

void Id::printNode(std::ostream& out) const {
    out << "Id( " << name << " )";
}

/* ==================== Result ==================== */

void Result::sem_analyze() {
    if (routine_returns_stack.empty()) {
        ErrorHandler::error(getLoc(), "Cannot use \"result\" keyword without a corresponding function definition");
        return;
    }
    const Type* current_ret_type = routine_returns_stack.back();
    if(!current_ret_type) {
        ErrorHandler::error(getLoc(), "Cannot use \"result\" keyword in a procedure definition");
        return;
    }
    type = current_ret_type->cloneType();
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

Value* Str::codegen() const {
    return CodeGen::Builder->CreateGlobalStringPtr(value);
}

/* ==================== Access ==================== */

void Access::sem_analyze() {
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

    auto const_index = index->getConstValue();
    if (const_index.has_value()) {
        int idx_val = const_index.value();
     
        if (idx_val < 0)
            ErrorHandler::error(index->getLoc(), "Array access: Index cannot be negative");
     
        else if (array_base_ptr->isComplete()) {
            if (idx_val >= array_base_ptr->getLength()) {
                ErrorHandler::error(index->getLoc(), "Array access: Index " + std::to_string(idx_val) + 
                                                     " is out of bounds (array size is " + 
                                                     std::to_string(array_base_ptr->getLength()) + ")");
            }
        }
    }
    type = array_base_ptr->getElementType()->cloneType();
}

void Access::printNode(std::ostream& out) const {
    out << "Access( " << *array_base << " [ " << *index << " ] )";
}

/* ==================== Deref ==================== */

void Deref::sem_analyze() {
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

std::optional<int> Const::getConstValue() const {
    return num;
}

Value* Const::codegen() const {
    return CodeGen::c32(num);
}

/* ==================== Bool ==================== */

void Bool::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Boolean);
}

void Bool::printNode(std::ostream& out) const {
    out << "Bool( " << val << " )";
}

Value* Bool::codegen() const {
    return llvm::ConstantInt::getBool(*CodeGen::TheContext, val);
}

/* ==================== Real ==================== */

void Real::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Real);
}

void Real::printNode(std::ostream& out) const {
    out << "Real( " << val << " )";
}

Value* Real::codegen() const {
    return CodeGen::f64(val);
}

/* ==================== Char ==================== */

void Char::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Char);
}

void Char::printNode(std::ostream& out) const {
    out << "Char( " << val << " )";
}

Value* Char::codegen() const {
    return llvm::ConstantInt::get(*CodeGen::TheContext, llvm::APInt(8, val, false));
}

/* ==================== Nil ==================== */

void Nil::sem_analyze() {
    type = std::make_unique<BasicType>(PCLtype::Nil);
}

void Nil::printNode(std::ostream& out) const {
    out << "Nil()";
}

Value* Nil::codegen() const {
    llvm::PointerType* nullPtrType = llvm::PointerType::getUnqual(*CodeGen::TheContext);
    return llvm::ConstantPointerNull::get(nullPtrType);
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

Value* Ref::codegen() const {
    return left_value->codegen();
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

std::optional<int> Unop::getConstValue() const {
    std::optional<int> c = expr->getConstValue();

    if (c.has_value()) {
        switch (op) {
            case UnOp::Plus:
                return c.value();
                break;
            case UnOp::Minus:
                return -c.value();
                break;
            default:
                return std::nullopt;
                break;
        }
    }
    return std::nullopt;
}

Value* Unop::codegen() const {
    Value* E = expr->codegen();

    if (expr->getExprType()->getType()==PCLtype::Real) {
        switch (op) {
        case UnOp::Plus:
            return E;
        case UnOp::Minus:
            return CodeGen::Builder->CreateFNeg(E, "fnegtmp");
        default:
            return nullptr;
        }
    } else {
        switch (op) {
        case UnOp::Plus:
            return E;
        case UnOp::Minus:
            return CodeGen::Builder->CreateNeg(E, "negtmp");
        case UnOp::Not:
            return CodeGen::Builder->CreateNot(E, "nottmp");
        default:
            return nullptr;
        }
    }
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

std::optional<int> Binop::getConstValue() const {
    std::optional<int> lconst = left->getConstValue();
    std::optional<int> rconst = right->getConstValue();

    if (lconst.has_value() && rconst.has_value()) {
        switch (op) {
        case BinOp::Add:
            return lconst.value()+rconst.value();
        case BinOp::Sub:
            return lconst.value()-rconst.value();
        case BinOp::Mul:
            return lconst.value()*rconst.value();
        case BinOp::Div:
            return lconst.value()/rconst.value();
        case BinOp::IntDiv:
            return lconst.value()/rconst.value();
        case BinOp::Mod:
            return lconst.value()%rconst.value();
        default:
            return std::nullopt;
        }
    }
    return std::nullopt;
}

Value* Binop::codegen() const {
    Value* L = left->codegen();
    Value* R = right->codegen();
    if (!L || !R) return nullptr;

    bool isLeftReal = (left->getExprType()->getType()==PCLtype::Real);
    bool isRightReal = (right->getExprType()->getType()==PCLtype::Real);
    bool isRealOp = (op==BinOp::Div) ? true : (isLeftReal || isRightReal);

    if (isRealOp) {
        // Casting operands to real
        if (!isLeftReal) L = CodeGen::Builder->CreateSIToFP(L, CodeGen::r64, "sit_to_fp_l");
        if (!isRightReal) R = CodeGen::Builder->CreateSIToFP(R, CodeGen::r64, "sit_to_fp_r");
        
        switch (op) {
        case BinOp::Add:
            return CodeGen::Builder->CreateFAdd(L, R, "faddtmp");
        case BinOp::Sub:
            return CodeGen::Builder->CreateFSub(L, R, "fsubtmp");
        case BinOp::Mul:
            return CodeGen::Builder->CreateFMul(L, R, "fmultmp");
        case BinOp::Div:
            return CodeGen::Builder->CreateFDiv(L, R, "fdivtmp");
        case BinOp::Eq:
            return CodeGen::Builder->CreateFCmpOEQ(L, R, "fcmpeqtmp");
        case BinOp::Neq:
            return CodeGen::Builder->CreateFCmpONE(L, R, "fcmpneqtmp");
        case BinOp::Lt:
            return CodeGen::Builder->CreateFCmpOLT(L, R, "fcmplttmp");
        case BinOp::Le:
            return CodeGen::Builder->CreateFCmpOLE(L, R, "fcmpletmp");
        case BinOp::Gt:
            return CodeGen::Builder->CreateFCmpOGT(L, R, "fcmpngttmp");
        case BinOp::Ge:
            return CodeGen::Builder->CreateFCmpOGE(L, R, "fcmpgetmp");
        default:
            return nullptr;
        }
    } else {
        switch (op) {
        case BinOp::Add:
            return CodeGen::Builder->CreateAdd(L, R, "addtmp");
        case BinOp::Sub:
            return CodeGen::Builder->CreateSub(L, R, "subtmp");
        case BinOp::Mul:
            return CodeGen::Builder->CreateMul(L, R, "multmp");
        case BinOp::IntDiv:
            return CodeGen::Builder->CreateSDiv(L, R, "sdivtmp");
        case BinOp::Mod:
            return CodeGen::Builder->CreateSRem(L, R, "smodtmp");
        case BinOp::And:
            return CodeGen::Builder->CreateAnd(L, R, "andtmp");
        case BinOp::Or:
            return CodeGen::Builder->CreateOr(L, R, "ortmp");
        case BinOp::Eq:
            return CodeGen::Builder->CreateICmpEQ(L, R, "icmpeqtmp");
        case BinOp::Neq:
            return CodeGen::Builder->CreateICmpNE(L, R, "icmpneqtmp");
        case BinOp::Lt:
            return CodeGen::Builder->CreateICmpSLT(L, R, "icmplttmp");
        case BinOp::Le:
            return CodeGen::Builder->CreateICmpSLE(L, R, "icmpletmp");
        case BinOp::Gt:
            return CodeGen::Builder->CreateICmpSGT(L, R, "icmpngttmp");
        case BinOp::Ge:
            return CodeGen::Builder->CreateICmpSGE(L, R, "icmpgetmp");
        default:
            return nullptr;
        }
    }    
}

/* ==================== CallProc ==================== */

void CallProc::sem_analyze() {
    for (const auto& arg : args) arg->sem_analyze();

    SymTabEntry* routine_entry = symtab.lookup(name);
    if (!routine_entry) {
        ErrorHandler::error(getLoc(), "Symbol " + name + " not declared");
        return;
    }
    
    RoutineName* routine_name = std::get_if<RoutineName>(&routine_entry->sym_info);
    if (!routine_name) {
        ErrorHandler::error(getLoc(), "Symbol " + name + " not declared as function or procedure");
        return;
    }

    if (routine_name->retType) {
        ErrorHandler::error(getLoc(), "Symbol " + name + " referes to a function not procedure");
        return;
    }

    std::vector<Parameter> pars = routine_name->parameters;
    if (args.size() != pars.size()) {
        std::ostringstream errorMsg;
        errorMsg << "Procedure \"" + name + "\" requires " << pars.size()
                 << "arguments but " << args.size() << " provided";
        ErrorHandler::error(getLoc(), errorMsg.str());
        return;
    }

    for (size_t i = 0; i < pars.size(); ++i) {
        const Type* arg_type = args[i]->getExprType();
        const Type* par_type = pars[i].type;
        
        if (!arg_type) continue;

        bool is_valid = false;
        std::string error_reason = "";

        if (pars[i].is_reference) {
            Lvalue* is_lval = dynamic_cast<Lvalue*>(args[i].get());
            if (!is_lval) {
                error_reason = "Argument must be an l-value (passed by reference).";
            } else {
                PointerType param_ptr(par_type->cloneType());
                PointerType arg_ptr(arg_type->cloneType());
                if (param_ptr.isCompatible(&arg_ptr)) {
                    is_valid = true;
                } else {
                    error_reason = "Type mismatch in pass-by-reference.";
                }
            }
        } else {
            if (par_type->isCompatible(arg_type)) {
                is_valid = true;
            } else {
                error_reason = "Type mismatch in pass-by-value.";
            }
        }

        if (!is_valid) {
            std::ostringstream errorMsg;
            errorMsg << "Argument in position " << (i + 1) << ": " << error_reason 
                     << " Expected: " << *par_type 
                     << ", but given: " << *arg_type;
            ErrorHandler::error(args[i]->getLoc(), errorMsg.str());
        }
    }
}

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

Value* CallProc::codegen() const {
    llvm::Function *CalleeF = CodeGen::TheModule->getFunction("writeString");
    llvm::Value* dummyString = CodeGen::Builder->CreateGlobalStringPtr("Hello from PCL CallProc!\n");

    // 3. Βάζουμε το argument στη λίστα
    std::vector<llvm::Value*> ArgsV;
    ArgsV.push_back(dummyString);

    // 4. Δημιουργούμε την κλήση στο LLVM IR
    return CodeGen::Builder->CreateCall(CalleeF, ArgsV);
}

/* ==================== CallFunc ==================== */

void CallFunc::sem_analyze() {
    for (const auto& arg : args) arg->sem_analyze();

    SymTabEntry* routine_entry = symtab.lookup(name);
    if (!routine_entry) {
        ErrorHandler::error(getLoc(), "Symbol " + name + " not declared");
        return;
    }
    
    RoutineName* routine_name = std::get_if<RoutineName>(&routine_entry->sym_info);
    if (!routine_name) {
        ErrorHandler::error(getLoc(), "Symbol " + name + " not declared as function or procedure");
        return;
    }

    if (routine_name->retType == nullptr) {
        ErrorHandler::error(getLoc(), "Symbol '" + name + "' refers to a procedure and cannot be used in an expression");
        return;
    }

    std::vector<Parameter> pars = routine_name->parameters;
    if (args.size() != pars.size()) {
        std::ostringstream errorMsg;
        errorMsg << "Procedure \"" + name + "\" requires " << pars.size()
                 << "arguments but " << args.size() << " provided";
        ErrorHandler::error(getLoc(), errorMsg.str());
        return;
    }

    for (size_t i = 0; i < pars.size(); ++i) {
        const Type* arg_type = args[i]->getExprType();
        const Type* par_type = pars[i].type;
        
        if (!arg_type) continue;

        bool is_valid = false;
        std::string error_reason = "";

        if (pars[i].is_reference) {
            Lvalue* is_lval = dynamic_cast<Lvalue*>(args[i].get());
            if (!is_lval) {
                error_reason = "Argument must be an l-value (passed by reference).";
            } else {
                PointerType param_ptr(par_type->cloneType());
                PointerType arg_ptr(arg_type->cloneType());
                if (param_ptr.isCompatible(&arg_ptr)) {
                    is_valid = true;
                } else {
                    error_reason = "Type mismatch in pass-by-reference.";
                }
            }
        } else {
            if (par_type->isCompatible(arg_type)) {
                is_valid = true;
            } else {
                error_reason = "Type mismatch in pass-by-value.";
            }
        }

        if (!is_valid) {
            std::ostringstream errorMsg;
            errorMsg << "Argument in position " << (i + 1) << ": " << error_reason 
                     << " Expected: " << *par_type 
                     << ", but given: " << *arg_type;
            ErrorHandler::error(args[i]->getLoc(), errorMsg.str());
        }
    }
    
    type = routine_name->retType->cloneType();
}

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

void Body::sem_analyze() {
    symtab.enterScope();
    
    for (const auto& local : locals)
        local->sem_analyze();
    block->sem_analyze();

    for (const auto& local : locals) {
        if (LabelDef* label_def = dynamic_cast<LabelDef*>(local.get())) {
            for (const auto& id : label_def->getIds()) {
                SymTabEntry* entry = symtab.lookup(id);
                if (entry) {
                    Label* lbl = std::get_if<Label>(&entry->sym_info);
                    if (lbl && !lbl->is_defined) {
                        ErrorHandler::error(local->getLoc(), "Label '"+id+"' was declared but never defined in the body");
                    }
                }
            }
        }
        else if (RoutineDef* routine_def = dynamic_cast<RoutineDef*>(local.get())) {
            if (routine_def->isForward()) {
                SymTabEntry* entry = symtab.lookup(routine_def->getName());
                if (entry) {
                    RoutineName* rtn = std::get_if<RoutineName>(&entry->sym_info);
                    if (rtn && rtn->is_forward) {
                        ErrorHandler::error(routine_def->getLoc(), "Routine '"+routine_def->getName()+"' was forward declared but never defined");
                    }
                }
            }
        }
    }
    
    symtab.exitScope();
}

void Body::printNode(std::ostream& out) const  {
    out << "Body(\n";
    for (const auto& local : locals) {
        out << "  " << *local << "\n";
    }
    out << "  " << *block << "\n)";
}

Value* Body::codegen() const {
    // TODO:: Locals codegen
    return block->codegen();
}

/* ==================== Var ==================== */

void Var::sem_analyze() {
    const Type* variables_type = type.get();
    if (!variables_type->isComplete()) {
        ErrorHandler::error(getLoc(), "Variable declarations must have a complete type");
        return;
    }
    for (const auto& id : ids)
        if (!symtab.insert(id, Variable(variables_type)))
            ErrorHandler::error(getLoc(), "Variable " + id + " is already declared in current scope");
}

void Var::printNode(std::ostream& out) const {
    out << "Var( names: ";
    for(const auto& i : ids)
        out << i << ", ";
    out << "type: " << *type << " )";
}

/* ==================== LabelDef ==================== */

const std::vector<std::string>& LabelDef::getIds() const {
    return ids;
}

void LabelDef::sem_analyze() {
    for (const auto& id : ids) 
        if (!symtab.insert(id, Label(false)))
            ErrorHandler::error(getLoc(), "Label " + id + " is already declared in current scope");
}

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

const Type* Formal::getFormalsType() const {
    return (const Type*)type.get();
}

const std::vector<std::string>& Formal::getIds() const {
    return ids;
}

bool Formal::isReference() const {
    return is_reference;
}

void Formal::sem_analyze() {
    const Type* formal_type = type.get();
    if (!is_reference && formal_type->getType() == PCLtype::Array) {
        ErrorHandler::error(getLoc(), "Arrays can only be passed by reference");
        return;
    }
    for (const auto& id : ids)
        if(!symtab.insert(id, Parameter(is_reference, formal_type)))
            ErrorHandler::error(getLoc(), "Current parameter id: " + id + " was previously declared in the current scope");
}

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

void Header::declare_parameters() {
    for (const auto& formal : formals)
        formal->sem_analyze();
}

/* ==================== RoutineDef ==================== */

bool RoutineDef::isForward() const {
    return (body==nullptr);
}

std::string RoutineDef::getName() const {
    return header->getName();
}

void RoutineDef::sem_analyze() {
    const std::string& name = header->getName();
    const Type* ret_type = header->getRetType();

    std::string funcproc = (ret_type) ? "Function " : "Procedure ";
    bool is_forward = (body==nullptr);

    if (ret_type!=nullptr && ret_type->getType()==PCLtype::Array) {
        ErrorHandler::error(getLoc(), "Subprograms are not allowed to have \"array\" as a return type");
        return;
    }    
    
    std::vector<Parameter> parameters;
    for (const auto& formal : header->getFormals()) {
        for (size_t i = 0; i < formal->getIds().size(); ++i) {
            parameters.push_back(Parameter(formal->isReference(), formal->getFormalsType()));
        }
    }

    RoutineName routine_name(is_forward, ret_type, parameters);
    
    if (!symtab.insert(name, routine_name)) {
        SymTabEntry* prev_routine_name_entry = symtab.lookup(name);
        if (prev_routine_name_entry) {
            if (RoutineName* prev_routine_name = std::get_if<RoutineName>(&prev_routine_name_entry->sym_info)) {               
                if (prev_routine_name->is_forward && !is_forward) {
                    bool definitions_match = true;
                    if ((prev_routine_name->retType == nullptr) != (ret_type == nullptr)) {
                        definitions_match = false;
                    } else if (ret_type != nullptr && !prev_routine_name->retType->isSame(ret_type)) {
                        definitions_match = false;
                    }
                    
                    if (prev_routine_name->parameters.size() != parameters.size()) {
                        definitions_match = false;
                    } else {
                        for (size_t i = 0; i < parameters.size(); ++i) {
                            if (prev_routine_name->parameters[i].is_reference != parameters[i].is_reference ||
                                !prev_routine_name->parameters[i].type->isSame(parameters[i].type)) {
                                definitions_match = false;
                                break;
                            }
                        }
                    }
                    if (definitions_match) {
                        prev_routine_name->is_forward = false;
                    } else {
                        ErrorHandler::error(getLoc(), funcproc + "'" + name + "' signature does not match its forward declaration");
                    }
                } else {
                    ErrorHandler::error(getLoc(), funcproc + "'" + name + "' was previously defined in the current scope");
                }
            } else {
                ErrorHandler::error(getLoc(), "'" + name + "' is already declared as a different symbol in the current scope");
            }
        }
    }
            
    if (!is_forward) {
        symtab.enterScope();
        header->declare_parameters();
        routine_returns_stack.push_back(ret_type);
        body->sem_analyze();
        routine_returns_stack.pop_back();
        symtab.exitScope();
    }
}

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

void Program::sem_analyze() {
    if (body) body->sem_analyze();
}

void Program::printNode(std::ostream& out) const {
    out << "Program( " << name << "\n" << *body << "\n)";
}

Value* Program::codegen() const {
    if (body) body->codegen();
    return nullptr;
}

void Program::compile(const CompilerOptions& co) {
    CodeGen::initialize("PCL Program");

    llvm::FunctionType *main_type = llvm::FunctionType::get(CodeGen::i32, {}, false);
    llvm::Function *main = llvm::Function::Create(
        main_type, llvm::Function::ExternalLinkage, "main", CodeGen::TheModule.get());

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*CodeGen::TheContext, "entry", main);
    CodeGen::Builder->SetInsertPoint(BB);

    codegen(); 
    
    CodeGen::Builder->CreateRet(CodeGen::c32(0));

    if (llvm::verifyModule(*CodeGen::TheModule, &llvm::errs())) {
        std::cerr << "Error: The generated LLVM IR is invalid!" << std::endl;
        CodeGen::TheModule->print(llvm::errs(), nullptr);
        std::exit(1);
    }

    if (co.optimize) CodeGen::optimize();

    CodeGen::generateOutput(co);
} 
