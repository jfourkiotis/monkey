#ifndef MONKEY_AST_H_INCLUDED
#define MONKEY_AST_H_INCLUDED

#include <string>
#include <memory>
#include <vector>
#include <numeric>
#include <initializer_list>
#include <cstdint>

#include "token.h"
#include "ast_visitor.h"

namespace ast {

#define ACCEPT_VISITOR(visitor) \
    void AcceptVisitor(AstVisitor &v) const override {\
        return v.Visit(*this);\
    }

// every AST node must implement the Node interface
class Node {
public:
    virtual ~Node() {}

    virtual void AcceptVisitor(AstVisitor &v) const = 0;
    virtual std::string TokenLiteral() const = 0;
    virtual std::string ToString() const = 0;
};//~ Node

class Statement : public Node {}; // dummy interface
class Expression: public Node {}; // dummy interface
    
    
using StatementList = std::vector<std::unique_ptr<Statement>>;

// The root node of every AST is the Program node
class Program final: public Node {
public:
    explicit Program(StatementList statements) : statements_(move(statements)) {}

    ACCEPT_VISITOR(AstVisitor);

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

    StatementList::size_type size() const { return statements_.size(); }

    const Statement* operator[](size_t index) const {
        return statements_[index].get();
    }
    
    const StatementList& Statements() const { return statements_; }

private:
    StatementList statements_;
};//~ Program

// To hold the identifier of a Let statement, we use the Identifier class
class Identifier final: public Expression {
public:
    Identifier(token::Token token, const std::string& value) : token_(token), value_(value) {}
    
    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }
    std::string Value() const { return value_; }
    std::string ToString() const override { return Value(); }
private:
    token::Token token_;
    std::string value_;
};//~ Identifier

// The let statement holds the name of the binding and the value of the 
// expression
class LetStatement final: public Statement {
public:
    LetStatement(token::Token token, std::unique_ptr<Identifier> name, std::unique_ptr<Expression> expr)
        : token_(token), name_(std::move(name)), expression_(std::move(expr)) {}
   
    ACCEPT_VISITOR(AstVisitor);

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

class ReturnStatement final: public Statement {
public:
    ReturnStatement(token::Token token, std::unique_ptr<Expression> expression)
        : token_(token), expression_(std::move(expression)) {}

    ACCEPT_VISITOR(AstVisitor);
    
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
    
    const Expression* Value() const { return expression_.get(); }
private:
    token::Token token_;
    std::unique_ptr<Expression> expression_;
};//~ ReturnStatement

class ExpressionStatement final: public Statement {
public:
    ExpressionStatement(token::Token token, std::unique_ptr<Expression> expression)
        : token_(token), expression_(std::move(expression)) {}

    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }
    
    std::string ToString() const override { return expression_ ? expression_->ToString() : ""; }

    const Expression* BorrowedExpression() const { return expression_.get(); }
private:
    token::Token token_;
    std::unique_ptr<Expression> expression_;
};//~ ExpressionStatement

class IntegerLiteral final: public Expression {
public:
    IntegerLiteral(token::Token token, int64_t value) : token_(token), value_(value) {}

    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override { return token_.literal; }

    int64_t Value() const { return value_; }
private:
    token::Token token_;
    int64_t value_;
};//~ IntegerLiteral

class PrefixExpression final: public Expression {
public:
    PrefixExpression(token::Token tok, const std::string& op, std::unique_ptr<Expression> right)
        : token_(tok), op_(op), right_(std::move(right)) {}

    ACCEPT_VISITOR(AstVisitor);

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

class InfixExpression final: public Expression {
public:
    InfixExpression(token::Token tok, std::unique_ptr<Expression> left, const std::string& op, std::unique_ptr<Expression> right)
        : token_(tok), left_(std::move(left)), op_(op), right_(std::move(right)) {}

    ACCEPT_VISITOR(AstVisitor);

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

class BooleanLiteral final: public Expression {
public:
    BooleanLiteral(token::Token tok, bool val) : token_(tok), value_(val) {}

    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override { return token_.literal; }

    bool Value() const { return value_; }
private:
    token::Token token_;
    bool value_;
};//~ Boolean

class BlockStatement final: public Statement {
public:
    BlockStatement(token::Token tok, StatementList statements)
        : token_(tok), statements_(std::move(statements)) {}

    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override { 
        std::string buf;
        
        for(auto& stmt : statements_) {
            buf.append(stmt->ToString());
        }

        return buf;
    }

    StatementList::size_type size() const { return statements_.size(); }

    const Statement* operator[](size_t index) const {
        return statements_[index].get();
    }
    
    const StatementList& Statements() const { return statements_; }
private:
    token::Token token_;
    StatementList statements_;
};

class IfExpression final: public Expression {
public:
    IfExpression(token::Token tok, std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> consequence, std::unique_ptr<BlockStatement> alternative)
        : token_(tok), condition_(std::move(condition)), consequence_(std::move(consequence)), alternative_(std::move(alternative)) {}

    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override {
        std::string buf;

        buf.append("if");
        buf.append(condition_->ToString());
        buf.append(1, ' ');
        buf.append(consequence_->ToString());

        if (alternative_) {
            buf.append("else ");
            buf.append(alternative_->ToString());
        }
        return buf;
    }

    const Expression* Condition() const {
        return condition_.get();
    }

    const BlockStatement* Consequence() const { 
        return consequence_.get();
    }

    const BlockStatement* Alternative() const {
        return alternative_.get();
    }

private:
    token::Token token_;
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<BlockStatement> consequence_;
    std::unique_ptr<BlockStatement> alternative_;
};//~ IfExpression

class FunctionLiteral final: public Expression {
public:
    using ParameterList = std::vector<std::unique_ptr<Identifier>>;
    FunctionLiteral(token::Token tok, ParameterList params, std::unique_ptr<BlockStatement> body)
        : token_(tok), parameters_(std::move(params)), body_(std::move(body)) {}

    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override {
        std::string buf;

        auto params = std::accumulate(
                parameters_.begin(),
                parameters_.end(),
                std::string(),
                [](const auto &accum, const auto &rhs) {
                    if (accum.empty()) return rhs->ToString();
                    return accum + ", " + rhs->ToString();
                });

        buf.append(TokenLiteral());
        buf.append(1, '(');
        buf.append(params);
        buf.append(1, ')');
        buf.append(body_->ToString());

        return buf;
    }

    const ParameterList& Parameters() const { return parameters_; }
    const BlockStatement* Body() const { return body_.get(); }
private:
    token::Token token_;
    ParameterList parameters_;
    std::unique_ptr<BlockStatement> body_;
};//~ FunctionLiteral

class CallExpression final: public Expression {
public:
    using ArgumentList = std::vector<std::unique_ptr<Expression>>;
    CallExpression(token::Token tok, std::unique_ptr<Expression> function, ArgumentList arguments)
        : token_(tok), function_(std::move(function)), arguments_(std::move(arguments)) {}

    ACCEPT_VISITOR(AstVisitor);

    std::string TokenLiteral() const override { return token_.literal; }

    std::string ToString() const override {
        std::string buf;
        
        auto args = std::accumulate(
                arguments_.begin(), 
                arguments_.end(), 
                std::string(), 
                [](const auto &accum, const auto &rhs) {
                    if (accum.empty()) return rhs->ToString(); 
                    return accum + ", " + rhs->ToString();
                });

        buf.append(function_->ToString());
        buf.append(1, '(');
        buf.append(args);
        buf.append(1, ')');

        return buf;
    }

    const Expression* Function() const { return function_.get(); }
    const ArgumentList& Arguments() const { return arguments_; }

private:
    token::Token token_;
    std::unique_ptr<Expression> function_;
    ArgumentList arguments_;
};//~ CallExpression

}//~ ast
#endif // MONKEY_AST_H_INCLUDED

