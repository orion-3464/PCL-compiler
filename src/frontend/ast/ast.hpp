#ifndef __AST_HPP__
#define __AST_HPP__

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <iostream>

#include "location.hpp"
#include "error.hpp"
#include "symbol.hpp"

typedef yy::location Loc;

class AST {
public:
    AST() = default;
    AST(Loc l) : loc(l) {}
    virtual ~AST() = default;
    Loc getLoc() const { return loc; }
    virtual void sem_analyze() {}
    virtual void printNode(std::ostream &out) const = 0;

private:
    Loc loc; 
};

inline std::ostream& operator << (std::ostream& out, const AST& ast) {
    ast.printNode(out);
    return out;
}

class Type : public AST {
public:
    Type() = default;
    Type(Loc l) : AST(l) {}
 
    virtual int getSize() const = 0;
    virtual bool isComposite() const = 0;
    virtual bool isComplete() const = 0;
    virtual bool isArithmetic() const = 0;
    virtual PCLtype inferType() const = 0;
    virtual PCLtype getType() const = 0;
    virtual std::unique_ptr<Type> cloneType() const = 0;
    virtual bool isSame(const Type* other) const = 0; // mainly for args
    virtual bool isCompatible(const Type* other) const = 0; // mainly for asgn
protected:
    PCLtype type;
};

class BasicType : public Type {
public:
    BasicType(PCLtype t) {type = t;}
    BasicType(PCLtype t, Loc l) : Type(l) {type = t;}
    
    int getSize() const override;
    bool isComposite() const override;
    bool isComplete() const override;
    bool isArithmetic() const override;
    PCLtype inferType() const override;
    PCLtype getType() const override;
    std::unique_ptr<Type> cloneType() const override;
    bool isSame(const Type* other) const override;
    bool isCompatible(const Type* other) const override;
    void printNode(std::ostream& out) const override;
};

class ArrayType : public Type {
public:
    ArrayType(int s, std::unique_ptr<Type> elem)
        : size(s), element_type(std::move(elem)) {
        type = PCLtype::Array;
    }
    ArrayType(int s, std::unique_ptr<Type> elem, Loc l)
        : Type(l), size(s), element_type(std::move(elem)) {
        type = PCLtype::Array;
    }

    const Type* getElementType() const;
    int getLength() const;

    int getSize() const override;
    bool isComposite() const override;
    bool isComplete() const override;
    bool isArithmetic() const override;
    PCLtype inferType() const override;
    PCLtype getType() const override;
    std::unique_ptr<Type> cloneType() const override;
    bool isSame(const Type* other) const override;
    bool isCompatible(const Type* other) const override;
    void printNode(std::ostream& out) const override;
private:
    int size; // Negative for incomplete types
    std::unique_ptr<Type> element_type;
};

class PointerType : public Type {
public:
    PointerType(std::unique_ptr<Type> t)
        : target(std::move(t)) {
        type = PCLtype::Pointer;
    }
    PointerType(std::unique_ptr<Type> tgt, Loc l) 
        : Type(l), target(std::move(tgt)) {
        type = PCLtype::Pointer;
    }

    const Type* getTargetType() const;
    
    int getSize() const override;
    bool isComposite() const override;
    bool isComplete() const override;
    bool isArithmetic() const override;
    PCLtype inferType() const override;
    PCLtype getType() const override;
    std::unique_ptr<Type> cloneType() const override;
    bool isSame(const Type* other) const override;
    bool isCompatible(const Type* other) const override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Type> target;
    PCLtype type = PCLtype::Pointer;
};

class Stmt : public AST {
public:
    Stmt(Loc l) : AST(l) {} 
};

class Expr : public AST {
public:
    Expr(Loc l) : AST(l) {}

    const Type* getExprType() { return type.get(); }
    void type_check(Type t);
protected:
    std::unique_ptr<Type> type;
};

class Lvalue : public Expr {
public:
    Lvalue(Loc l) : Expr(l) {}
};

class Rvalue : public Expr {
public:
    Rvalue(Loc l) : Expr(l) {}
};

class Block : public Stmt {
public:
    Block(Loc l) : Stmt(l) {}

    void append(std::unique_ptr<Stmt> s);
    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::vector<std::unique_ptr<Stmt>> stmts;
};

class Empty : public Stmt {
public:
    Empty(Loc l) : Stmt(l) {}

    void printNode(std::ostream& out) const override;
};

class Assignment : public Stmt {
public:
    Assignment(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, Loc l)
        : Stmt(l), lval(std::move(lhs)), e(std::move(rhs)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> lval;
    std::unique_ptr<Expr> e;
};

class Conditional : public Stmt {
public:
    Conditional(std::unique_ptr<Expr> c,
                std::unique_ptr<Stmt> ts,
                std::unique_ptr<Stmt> es,
                Loc l)
        : Stmt(l), cond(std::move(c)), then_stmt(std::move(ts)), else_stmt(std::move(es)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> then_stmt;
    std::unique_ptr<Stmt> else_stmt;
};

class While : public Stmt {
public:
    While(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> lo, Loc l)
        : Stmt(l), cond(std::move(c)), loop(std::move(lo)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> loop;
};

class Labeling : public Stmt {
public:
    Labeling(std::string la, std::unique_ptr<Stmt> c, Loc l)
        : Stmt(l), label(la), code(std::move(c)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::string label;
    std::unique_ptr<Stmt> code;
};

class Goto : public Stmt {
public:
    Goto(std::string la, Loc l) : Stmt(l), label(la) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::string label;
};

class Return : public Stmt {
public:
    Return(Loc l) : Stmt(l) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
};

class New : public Stmt {
public:
    New(std::unique_ptr<Expr> n, std::unique_ptr<Expr> e, Loc l)
        : Stmt(l), size(std::move(n)), lvl(std::move(e)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> size;
    std::unique_ptr<Expr> lvl;
};

class Dispose : public Stmt {
public:
    Dispose(bool b, std::unique_ptr<Expr> e, Loc l)
        : Stmt(l), brackets(b), lvl(std::move(e)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    bool brackets;
    std::unique_ptr<Expr> lvl;
};

class Id : public Lvalue {
public:
    Id(std::string n, Loc l) : Lvalue(l), name(n) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::string name;
};

class Result : public Lvalue {
public:
    Result(Loc l) : Lvalue(l) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
};

class Str : public Lvalue {
public:
    Str(std::string v, Loc l) : Lvalue(l), value(v) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::string value;
};

class Access : public Lvalue {
public:
    Access(std::unique_ptr<Expr> a, std::unique_ptr<Expr> idx, Loc l)
        : Lvalue(l), array_base(std::move(a)), index(std::move(idx)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> array_base;
    std::unique_ptr<Expr> index;
};

class Deref : public Lvalue {
public:
    Deref(std::unique_ptr<Expr> p, Loc l)
        : Lvalue(l), pointer(std::move(p)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> pointer;
};

class Const : public Rvalue {
public:
    Const(unsigned long n, Loc l) : Rvalue(l), num(n) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    unsigned long num;
};

class Bool : public Rvalue {
public:
    Bool(bool v, Loc l) : Rvalue(l), val(v) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    bool val;
};

class Real : public Rvalue {
public:
    Real(double v, Loc l) : Rvalue(l), val(v) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    double val;
};

class Char : public Rvalue {
public:
    Char(char v, Loc l) : Rvalue(l), val(v) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    char val;
};

class Nil : public Rvalue {
public:
    Nil(Loc l) : Rvalue(l) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
};

class Ref : public Rvalue {
public:
    Ref(std::unique_ptr<Expr> le, Loc l) : Rvalue(l), left_value(std::move(le)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> left_value;
};

enum class BinOp {
    Add, Sub, Mul,   
    Div, IntDiv, Mod,    
    And, Or,     
    Eq,  Neq,    
    Lt,  Le, Gt, Ge     
};

enum class UnOp {
    Not,    
    Plus,
    Minus
};

std::string to_string(BinOp op);
std::string to_string(UnOp op);

class Unop : public Rvalue {
public:
    Unop(UnOp o, std::unique_ptr<Expr> e, Loc l)
        : Rvalue(l), op(o), expr(std::move(e)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    UnOp op;
    std::unique_ptr<Expr> expr;
};

class Binop : public Rvalue {
public:
    Binop(std::unique_ptr<Expr> le,
          BinOp o,
          std::unique_ptr<Expr> re,
          Loc l)
        : Rvalue(l), left(std::move(le)), op(o), right(std::move(re)) {}

    void sem_analyze() override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> left;
    BinOp op;
    std::unique_ptr<Expr> right;
};

class CallProc : public Stmt {
public:
    CallProc(std::string n,
             std::vector<std::unique_ptr<Expr>> a,
             Loc l) : Stmt(l), name(n), args(std::move(a)) {}
    
    void printNode(std::ostream& out) const override;
private:
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
};

class CallFunc : public Rvalue {
public:
    CallFunc(std::string n,
             std::vector<std::unique_ptr<Expr>> a,
             Loc l) : Rvalue(l), name(n), args(std::move(a)) {}

    void printNode(std::ostream& out) const override;
private:
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
};

class Local : public AST {
public:
    Local(Loc l) : AST(l) {} 
};

class Body : public AST {
public:
    Body(std::vector<std::unique_ptr<Local>> locs,
         std::unique_ptr<Block> blk,
         Loc l)
        : AST(l), locals(std::move(locs)), block(std::move(blk)) {}

    void printNode(std::ostream& out) const override;
private:
    std::vector<std::unique_ptr<Local>> locals;
    std::unique_ptr<Block> block;
};

class Var : public Local {
public:
    Var(std::vector<std::string> i,
        std::unique_ptr<Type> t,
        Loc l) : Local(l), type(std::move(t)) { ids = i; }

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Type> type;
    std::vector<std::string> ids;
};

class LabelDef : public Local {
public:
    LabelDef(std::vector<std::string> i, Loc l)
        : Local(l) { ids = i; }

    void printNode(std::ostream& out) const override;
private:
    std::vector<std::string> ids;
};

class Formal : public AST {
public:
    Formal(std::vector<std::string> i,
           std::unique_ptr<Type> t,
           bool is_ref,
           Loc l) 
        : AST(l), ids(std::move(i)), type(std::move(t)), is_reference(is_ref) {}

    void printNode(std::ostream& out) const override;

private:
    std::vector<std::string> ids;
    std::unique_ptr<Type> type;
    bool is_reference;
};

class Header {
public:
    Header(std::string n, std::vector<std::unique_ptr<Formal>> f,
           std::unique_ptr<Type> t) : name(n), formals(std::move(f)),
                                      return_type(std::move(t)) {}
    
    const std::string& getName() const;
    const std::vector<std::unique_ptr<Formal>>& getFormals() const;
    const Type* getRetType() const;
private:
    std::string name;
    std::vector<std::unique_ptr<Formal>> formals;
    std::unique_ptr<Type> return_type; // nullptr για procedure
};

class RoutineDef : public Local {
public:
    RoutineDef(std::unique_ptr<Header> h,
               std::unique_ptr<Body> b,        // nullptr for forward declaration
               Loc l)                          // and header
        : Local(l), header(std::move(h)), body(std::move(b)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Header> header;
    std::unique_ptr<Body> body; 
};

class Program : public AST {
public:
    Program(std::string n, std::unique_ptr<Body> b, Loc l)
        : AST(l), name(n), body(std::move(b)) {}

    void printNode(std::ostream& out) const override;
private:
    std::string name;
    std::unique_ptr<Body> body;
};

#endif /* __AST_HPP__ */
