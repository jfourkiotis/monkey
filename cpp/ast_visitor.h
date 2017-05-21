#ifndef MONKEY_AST_VISITOR_H_INCLUDED
#define MONKEY_AST_VISITOR_H_INCLUDED

#include <memory>

namespace ast {

class Program;
class Identifier;
class LetStatement;
class ReturnStatement;
class ExpressionStatement;
class PrefixExpression;
class InfixExpression;
class IntegerLiteral;
class BooleanLiteral;
class BlockStatement;
class IfExpression;
class FunctionLiteral;
class CallExpression;

class AstVisitor {
public:
    virtual ~AstVisitor() {}
    virtual void Visit(const Program &node) = 0;
    virtual void Visit(const Identifier& node) = 0;
    virtual void Visit(const LetStatement& node) = 0;
    virtual void Visit(const ReturnStatement& node) = 0;
    virtual void Visit(const ExpressionStatement& node) = 0;
    virtual void Visit(const PrefixExpression& node) = 0;
    virtual void Visit(const InfixExpression& node) = 0;
    virtual void Visit(const IntegerLiteral& node) = 0;
    virtual void Visit(const BooleanLiteral& node) = 0;
    virtual void Visit(const BlockStatement& node) = 0;
    virtual void Visit(const IfExpression& node) = 0;
    virtual void Visit(const FunctionLiteral& node) = 0;
    virtual void Visit(const CallExpression& node) = 0;
};//~ AstVisitor

}//~ ast


#endif // MONKEY_AST_VISITOR_H_INCLUDED 

