#ifndef MONKEY_AST_H_INCLUDED
#define MONKEY_AST_H_INCLUDED

#include <string>
#include <memory>
#include <vector>

#include "token.h"

namespace ast {
// every AST node must implement the Node interface
class Node {
public:
    virtual ~Node() {}
    virtual std::string TokenLiteral() const = 0;
};//~ Node

class Statement : public Node {}; // dummy interface
class Expression: public Node {}; // dummy interface

// The root node of every AST is the Program node
class Program final: public Node {
public:
    using Statements = std::vector<std::unique_ptr<Statement>>;
    explicit Program(Statements statements) : statements_(move(statements)) {}

    std::string TokenLiteral() const override {
        return statements_.empty() ? "" : statements_[0]->TokenLiteral();
    }

    Statements::size_type size() const { return statements_.size(); }

    Statement* operator[](size_t index) {
        return statements_[index].get();
    }
private:
    Statements statements_;
};//~ Program

// To hold the identifier of a Let statement, we use the Identifier class
class Identifier : public Expression {
public:
    Identifier(token::Token token, const std::string& value) : token_(token), value_(value) {}
    std::string TokenLiteral() const override { return token_.literal; }
    std::string Value() const { return value_; }
private:
    token::Token token_;
    std::string value_;
};//~ Identifier

// The let statement holds the name of the binding and the value of the 
// expression
class LetStatement : public Statement {
public:
    LetStatement(token::Token token, std::unique_ptr<Identifier> name, std::unique_ptr<Expression> expr)
        : token_(token), name_(std::move(name)), expression_(std::move(expr)) {}
    
    std::string TokenLiteral() const override { return token_.literal; }
    const Identifier* Name() const { return name_.get(); }
private:
    token::Token token_;
    std::unique_ptr<Identifier> name_;
    std::unique_ptr<Expression> expression_;
};//~ LetStatement

}//~ ast
#endif // MONKEY_AST_H_INCLUDED

