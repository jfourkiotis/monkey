#ifndef MONKEY_EVAL_H_INCLUDED
#define MONKEY_EVAL_H_INCLUDED

#include <memory>
#include <vector>
#include <cassert>

#include "ast.h"
#include "ast_visitor.h"
#include "object.h"

std::shared_ptr<MObject> Eval(const ast::Node& node);

namespace {

using namespace ast;
    
std::shared_ptr<MBoolean> M_TRUE = std::make_shared<MBoolean>(true);
std::shared_ptr<MBoolean> M_FALSE= std::make_shared<MBoolean>(false);
std::shared_ptr<MNull> M_NULL = std::make_shared<MNull>();

class EvalVisitor final : public AstVisitor {
public:

    void Visit(const Program &node) override {
        _EvalStatements(node.Statements());
    };

    void Visit(const Identifier& node) override {
    };

    void Visit(const LetStatement& node) override {
    }

    void Visit(const ReturnStatement& node) override {
    }

    void Visit(const ExpressionStatement& node) override {
        node.BorrowedExpression()->AcceptVisitor(*this);
    }

    void Visit(const PrefixExpression& node) override {
        auto expression = node.Right();
        result.push_back(_EvalPrefixExpression(node.Operator(), Eval(*expression)));
    }

    void Visit(const InfixExpression& node) override {
    }
    
    void Visit(const IntegerLiteral& node) override {
        result.push_back(std::make_shared<MInteger>(node.Value()));
    };

    void Visit(const BooleanLiteral& node) override {
        if (node.Value()) {
            result.push_back(M_TRUE);
        } else {
            result.push_back(M_FALSE);
        }
    }

    void Visit(const BlockStatement& node) override {
    }

    void Visit(const IfExpression& node) override {
    }

    void Visit(const FunctionLiteral& node) override  {
    }

    void Visit(const CallExpression& node) override {
    }

    std::shared_ptr<MObject> Result() const { 
        assert(result.size() == 1);
        return result.back(); 
    }
private:
    void _EvalStatements(const Program::StatementList& statements) {
        std::vector<std::shared_ptr<MObject>> tmp(std::move(result));
        for(auto &&stmt : statements) {
            stmt->AcceptVisitor(*this);
        }
        
        if (!result.empty()) {
            tmp.push_back(result.back());
        }
        result.swap(tmp);
    }
    
    std::shared_ptr<MObject> _EvalPrefixExpression(const std::string& op, const std::shared_ptr<MObject>& obj) {
        if (op == "!") {
            return _EvalBangOperatorExpression(obj);
        } else if (op == "-") {
            return _EvalMinusPrefixOperatorExpression(obj);
        }
        return M_NULL;
    }
    
    std::shared_ptr<MObject> _EvalBangOperatorExpression(const std::shared_ptr<MObject>& obj) {
        if (obj == M_TRUE) {
            return M_FALSE;
        } else if (obj == M_FALSE) {
            return M_TRUE;
        } else if (obj == M_NULL) {
            return M_TRUE;
        } else {
            return M_FALSE;
        }
    }
    
    std::shared_ptr<MObject> _EvalMinusPrefixOperatorExpression(const std::shared_ptr<MObject> & obj) {
        
        if (obj->Type() != ObjectType::INTEGER_OBJ) {
            return M_NULL;
        }
        auto i = std::static_pointer_cast<MInteger>(obj);
        return std::make_shared<MInteger>(-i->Value());
    }
    
    std::vector<std::shared_ptr<MObject>> result;

};//~ EvalVisitor

}

std::shared_ptr<MObject> Eval(const ast::Node& node) {
    EvalVisitor visitor;
    node.AcceptVisitor(visitor);
    return visitor.Result();
}

#endif
