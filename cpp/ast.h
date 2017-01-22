#ifndef MONKEY_AST_H_INCLUDED
#define MONKEY_AST_H_INCLUDED

#include <string>
#include <memory>
#include <vector>
#include <initializer_list>
#include <cstdint>

#include "token.h"

namespace ast {
// every AST node must implement the Node interface
class Node {
public:
    virtual ~Node() {}
    virtual std::string TokenLiteral() const = 0;
    virtual std::string ToString() const = 0;
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

    std::string ToString() const override {
        std::string buf;
        for (const auto& stmt : statements_) {
            buf.append(stmt->ToString());
        }
        return buf;
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
    std::string ToString() const override { return Value(); }
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

    std::string ToString() const override {
        std::string buf;

        buf.append(TokenLiteral());
        buf.append(1, ' ');
        buf.append(Name()->ToString());
        buf.append(" = ");

        if (expression_) {
            buf.append(expression_->ToString());
        }

        buf.append(1, ';');
        return buf;
    }
    const Identifier* Name() const { return name_.get(); }
private:
    token::Token token_;
    std::unique_ptr<Identifier> name_;
    std::unique_ptr<Expression> expression_;
};//~ LetStatement

class ReturnStatement : public Statement {
public:
    ReturnStatement(token::Token token, std::unique_ptr<Expression> expression)
        : token_(token), expression_(std::move(expression)) {}
    
    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override {
        std::string buf;

        buf.append(TokenLiteral());
        buf.append(1, ' ');
        if (expression_) {
            buf.append(expression_->ToString());
        }

        buf.append(1, ';');
        return buf;
    }
private:
    token::Token token_;
    std::unique_ptr<Expression> expression_;
};//~ ReturnStatement

class ExpressionStatement : public Statement {
public:
    ExpressionStatement(token::Token token, std::unique_ptr<Expression> expression)
        : token_(token), expression_(std::move(expression)) {}

    std::string TokenLiteral() const override { return token_.literal; }
    
    std::string ToString() const override { return expression_ ? expression_->ToString() : ""; }

    const Expression* BorrowedExpression() const { return expression_.get(); }
private:
    token::Token token_;
    std::unique_ptr<Expression> expression_;
};//~ ExpressionStatement

class IntegerLiteral : public Expression {
public:
    IntegerLiteral(token::Token token, int64_t value) : token_(token), value_(value) {}

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override { return token_.literal; }

    int64_t Value() const { return value_; }
private:
    token::Token token_;
    int64_t value_;
};//~ IntegerLiteral

class PrefixExpression : public Expression {
public:
    PrefixExpression(token::Token tok, const std::string& op, std::unique_ptr<Expression> right)
        : token_(tok), op_(op), right_(std::move(right)) {}

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override { 
        std::string buf;

        buf.append(1, '(');
        buf.append(op_);
        buf.append(right_->ToString());
        buf.append(1, ')');

        return buf;
    }

    std::string Operator() const { return op_; }

    const Expression* Right() const { return right_.get(); }
private:
    token::Token token_;
    std::string op_;
    std::unique_ptr<Expression> right_;
};//~ PrefixExpression

class InfixExpression : public Expression {
public:
    InfixExpression(token::Token tok, std::unique_ptr<Expression> left, const std::string& op, std::unique_ptr<Expression> right)
        : token_(tok), left_(std::move(left)), op_(op), right_(std::move(right)) {}

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override {
        std::string buf;

        buf.append(1, '(');
        buf.append(left_->ToString());
        buf.append(1, ' ');
        buf.append(op_);
        buf.append(1, ' ');
        buf.append(right_->ToString());
        buf.append(1, ')');

        return buf;
    }

    std::string Operator() const { return op_; }

    const Expression* Left() const { return left_.get(); }
    const Expression* Right() const { return right_.get(); }
private:
    token::Token token_;
    std::unique_ptr<Expression> left_;
    std::string op_;
    std::unique_ptr<Expression> right_;
};//~ InfixExpression

class Boolean : public Expression {
public:
    Boolean(token::Token tok, bool val) : token_(tok), value_(val) {}

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override { return token_.literal; }

    bool Value() const { return value_; }
private:
    token::Token token_;
    bool value_;
};//~ Boolean

}//~ ast
#endif // MONKEY_AST_H_INCLUDED

