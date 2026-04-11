#ifndef __AST_HPP__
#define __AST_HPP__

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <iostream>
#include "location.hpp"

class AST {
public:
    virtual ~AST() = default;
    virtual void printNode(std::ostream &out) const = 0;
};

inline std::ostream& operator << (std::ostream& out, const AST& ast) {
    ast.printNode(out);
    return out;
} 

class Stmt : public AST {
};

class Expr : public AST {
};

class Type : public AST {
};

class Local : public AST {
};

class Block : public Stmt {
public:
    Block() = default;

    void append(std::unique_ptr<Stmt> s);
    void printNode(std::ostream& out) const override;
private:
    std::vector<std::unique_ptr<Stmt>> stmts;
};

class Assignment : public Stmt {
public:
    Assignment(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : lval(std::move(lhs)), e(std::move(rhs)) {}

    void printNode(std::ostream& out) const override;

private:
    std::unique_ptr<Expr> lval;
    std::unique_ptr<Expr> e;
};

class Call : public AST {
};

class Conditional : public Stmt {
public:
    Conditional(std::unique_ptr<Expr> c,
                std::unique_ptr<Stmt> ts,
                std::unique_ptr<Stmt> es)
        : cond(std::move(c)), then_stmt(std::move(ts)), else_stmt(std::move(es)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> then_stmt;
    std::unique_ptr<Stmt> else_stmt;
};

class While : public Stmt {
public:
    While(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> l)
        : cond(std::move(c)), loop(std::move(l)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> loop;
};

class Labeling : public Stmt {
public:
    Labeling(std::string l, std::unique_ptr<Stmt> c)
        : label(l), code(std::move(c)) {}
    
    void printNode(std::ostream& out) const override;
private:
    std::string label;
    std::unique_ptr<Stmt> code;
};

class Goto : public Stmt {
public:
    Goto(std::string l) : label(l) {}

    void printNode(std::ostream& out) const override;
private:
    std::string label;
};

class New : public Stmt {
public:
    New(std::unique_ptr<int> n, std::unique_ptr<Expr> e)
        : size(std::move(n)), lvl(std::move(e)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<int> size;
    std::unique_ptr<Expr> lvl;
};

class Dispose : public Stmt {
public:
    Dispose(std::unique_ptr<Expr> e)
        : lvl(std::move(e)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> lvl;
};

class Lvalue : public Expr {};
class Rvalue : public Expr {};

class Id : public Lvalue {
public:
    Id(std::string n) : name(n) {}

    void printNode(std::ostream& out) const override;
private:
    std::string name;
};

class Str : public Lvalue {
public:
    Str(std::string v) : value(v) {}

    void printNode(std::ostream& out) const override;
private:
    std::string value;
};

class Access : public Lvalue {
public:
    Access(std::unique_ptr<Expr> a, std::unique_ptr<Expr> idx)
        : array_base(std::move(a)), index(std::move(idx)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> array_base;
    std::unique_ptr<Expr> index;
};

class Deref : public Lvalue {
public:
    Deref(std::unique_ptr<Expr> p)
        : pointer(std::move(p)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> pointer;
};

class Const : public Rvalue {
public:
    Const(int n) : num(n) {}

    void printNode(std::ostream& out) const override;
private:
    int num;
};

class Bool : public Rvalue {
public:
    Bool(std::string v) : val(v) {}

    void printNode(std::ostream& out) const override;
private:
    std::string val;
};

class Real : public Rvalue {
public:
    Real(double v) : val(v) {}

    void printNode(std::ostream& out) const override;
private:
    double val;
};

class Char : public Rvalue {
public:
    Char(char v) : val(v) {}
    
    void printNode(std::ostream& out) const override;
private:
    char val;
};

class Ref : public Rvalue {
public:
    Ref(std::unique_ptr<Expr> l) : left_value(std::move(l)) {}
    
    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> left_value;
};

class Unop : public Rvalue {
public:
    Unop(std::string o, std::unique_ptr<Expr> e)
        : op(o), expr(std::move(e)) {}

    void printNode(std::ostream& out) const override;
private:
    std::string op;
    std::unique_ptr<Expr> expr;
};

class Binop : public Rvalue {
public:
    Binop(std::unique_ptr<Expr> le, std::string o, std::unique_ptr<Expr> re)
        : left(std::move(le)), op(o), right(std::move(re)) {}

    void printNode(std::ostream& out) const override;
private:
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
};

class Body : public AST {
};




#endif /* __AST_HPP__ */
