#ifndef __AST_HPP__
#define __AST_HPP__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

class AST {
};

class Program : public AST {
};

class Body : public AST {
};

class Local : public AST {
};

class Definition : public AST {
};

class Id : public AST {
};

class Header : public AST {
};

class Formal : public AST {
};

class Type : public AST {
};

class Block : public AST {
};

class Stmt : public AST {
};

class Expr : public AST {
};

class Call : public AST {
};

#endif /* __AST_HPP__ */
