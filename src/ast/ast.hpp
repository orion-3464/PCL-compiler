#ifndef __AST_HPP__
#define __AST_HPP__

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <iostream>
#include "location.hpp"

typedef yy::location Loc;

class AST {
public:
    AST(Loc l) : loc(l) {}
    virtual ~AST() = default;
    virtual void printNode(std::ostream &out) const = 0;

private:
    Loc loc; 
};

inline std::ostream& operator << (std::ostream& out, const AST& ast) {
    ast.printNode(out);
    return out;
} 

class Stmt : public AST {
public:
    Stmt(Loc l) : AST(l) {} 
};

class Expr : public AST {
public:
    Expr(Loc l) : AST(l) {}
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
    Assignment(std::unique_ptr<Lvalue> lhs, std::unique_ptr<Expr> rhs, Loc l)
        : Stmt(l), lval(std::move(lhs)), e(std::move(rhs)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Lvalue> lval;
    std::unique_ptr<Expr> e;
};

class Conditional : public Stmt {
public:
    Conditional(std::unique_ptr<Expr> c,
                std::unique_ptr<Stmt> ts,
                std::unique_ptr<Stmt> es,
                Loc l)
        : Stmt(l), cond(std::move(c)), then_stmt(std::move(ts)), else_stmt(std::move(es)) {}

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
    
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> loop;
};

class Labeling : public Stmt {
public:
    Labeling(std::string la, std::unique_ptr<Stmt> c, Loc l)
        : Stmt(l), label(la), code(std::move(c)) {}
    
    void printNode(std::ostream& out) const override;
private:
    std::string label;
    std::unique_ptr<Stmt> code;
};

class Goto : public Stmt {
public:
    Goto(std::string la, Loc l) : Stmt(l), label(la) {}

    void printNode(std::ostream& out) const override;
private:
    std::string label;
};

class Return : public Stmt {
public:
    Return(Loc l) : Stmt(l) {}

    void printNode(std::ostream& out) const override;
};

class New : public Stmt {
public:
    New(std::unique_ptr<Expr> n, std::unique_ptr<Expr> e, Loc l)
        : Stmt(l), size(std::move(n)), lvl(std::move(e)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> size;
    std::unique_ptr<Expr> lvl;
};

class Dispose : public Stmt {
public:
    Dispose(std::unique_ptr<Expr> e, Loc l)
        : Stmt(l), lvl(std::move(e)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> lvl;
};

class Id : public Lvalue {
public:
    Id(std::string n, Loc l) : Lvalue(l), name(n) {}

    void printNode(std::ostream& out) const override;
private:
    std::string name;
};

class Result : public Lvalue {
public:
    Result(Loc l) : Lvalue(l) {}

    void printNode(std::ostream& out) const override;
};

class Str : public Lvalue {
public:
    Str(std::string v, Loc l) : Lvalue(l), value(v) {}

    void printNode(std::ostream& out) const override;
private:
    std::string value;
};

class Access : public Lvalue {
public:
    Access(std::unique_ptr<Expr> a, std::unique_ptr<Expr> idx, Loc l)
        : Lvalue(l), array_base(std::move(a)), index(std::move(idx)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> array_base;
    std::unique_ptr<Expr> index;
};

class Deref : public Lvalue {
public:
    Deref(std::unique_ptr<Expr> p, Loc l)
        : Lvalue(l), pointer(std::move(p)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> pointer;
};

class Const : public Rvalue {
public:
    Const(int n, Loc l) : Rvalue(l), num(n) {}

    void printNode(std::ostream& out) const override;
private:
    int num;
};

class Bool : public Rvalue {
public:
    Bool(std::string v, Loc l) : Rvalue(l), val(v) {}

    void printNode(std::ostream& out) const override;
private:
    std::string val;
};

class Real : public Rvalue {
public:
    Real(double v, Loc l) : Rvalue(l), val(v) {}

    void printNode(std::ostream& out) const override;
private:
    double val;
};

class Char : public Rvalue {
public:
    Char(char v, Loc l) : Rvalue(l), val(v) {}
    
    void printNode(std::ostream& out) const override;
private:
    char val;
};

class Nil : public Rvalue {
public:
    Nil(Loc l) : Rvalue(l) {}

    void printNode(std::ostream& out) const override;
};

class Ref : public Rvalue {
public:
    Ref(std::unique_ptr<Expr> le, Loc l) : Rvalue(l), left_value(std::move(le)) {}
    
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> left_value;
};

class Unop : public Rvalue {
public:
    Unop(std::string o, std::unique_ptr<Expr> e, Loc l)
        : Rvalue(l), op(o), expr(std::move(e)) {}

    void printNode(std::ostream& out) const override;
private:
    std::string op;
    std::unique_ptr<Expr> expr;
};

class Binop : public Rvalue {
public:
    Binop(std::unique_ptr<Expr> le,
          std::string o,
          std::unique_ptr<Expr> re,
          Loc l)
        : Rvalue(l), left(std::move(le)), op(o), right(std::move(re)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> left;
    std::string op;
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

class CallFunc : public Expr {
public:
    CallFunc(std::string n,
             std::vector<std::unique_ptr<Expr>> a,
             Loc l) : Expr(l), name(n), args(std::move(a)) {}

    void printNode(std::ostream& out) const override;
private:
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
};

class Type : public AST {
public:
    Type(Loc l) : AST(l) {}

    virtual int getSize() const = 0;
    virtual bool isComplete() const = 0;
    virtual bool isArithmetic() const = 0;
};

class BasicType : public Type {
public:
    BasicType(std::string n, Loc l) : Type(l), name(n) {}

    int getSize() const override;
    bool isComplete() const override;
    bool isArithmetic() const override;
    void printNode(std::ostream& out) const override;
private:
    std::string name; 
};

class ArrayType : public Type {
public:
    ArrayType(int s, std::unique_ptr<Type> elem, Loc l)
        : Type(l), size(s), element_type(std::move(elem)) {}

    int getSize() const override;
    bool isComplete() const override;
    bool isArithmetic() const override;
    void printNode(std::ostream& out) const override;
private:
    int size; 
    std::unique_ptr<Type> element_type;
};

class PointerType : public Type {
public:
    PointerType(std::unique_ptr<Type> tgt, Loc l) 
        : Type(l), target(std::move(tgt)) {}

    int getSize() const override;
    bool isComplete() const override;
    bool isArithmetic() const override;
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Type> target;
};

class Local : public AST {
    Local(Loc l) : AST(l) {} 
};


class Body : public AST {
};


#endif /* __AST_HPP__ */
