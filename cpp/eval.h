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
        auto left = Eval(*node.Left());
        auto right= Eval(*node.Right());
        result.push_back(_EvalInfixExpression(node.Operator(), left, right));
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
        _EvalStatements(node.Statements());
    }

    void Visit(const IfExpression& node) override {
        auto condition = Eval(*node.Condition());
        if (_IsTruthy(condition)) {
            result.push_back(Eval(*node.Consequence()));
        } else if (node.Alternative()) {
            result.push_back(Eval(*node.Alternative()));
        } else {
            result.push_back(M_NULL);
        }
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
    bool _IsTruthy(const std::shared_ptr<MObject>& obj) const {
        if (obj == M_NULL) {
            return false;
        } else if (obj == M_TRUE) {
            return true;
        } else if (obj == M_FALSE) {
            return false;
        } else {
            return true;
        }
    }
    
    void _EvalStatements(const StatementList& statements) {
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
    
    std::shared_ptr<MObject> _EvalInfixExpression(const std::string& op, const std::shared_ptr<MObject>& left, const std::shared_ptr<MObject>& right) {
        if (left->Type() == ObjectType::INTEGER_OBJ && right->Type() == ObjectType::INTEGER_OBJ) {
            return _EvalIntegerInfixExpression(op, left, right);
        } else if (op == "==") {
            return left == right ? M_TRUE : M_FALSE;
        } else if (op == "!=") {
            return left != right ? M_TRUE : M_FALSE;
        } else {
            return M_NULL;
        }
    }
    
    std::shared_ptr<MObject> _EvalIntegerInfixExpression(const std::string& op, const std::shared_ptr<MObject>& left, const std::shared_ptr<MObject>& right) {
        const std::shared_ptr<MInteger>& lv = std::static_pointer_cast<MInteger>(left);
        const std::shared_ptr<MInteger>& rv = std::static_pointer_cast<MInteger>(right);
        if (op == "+") {
            return std::make_shared<MInteger>(lv->Value() + rv->Value());
        } else if (op == "-") {
            return std::make_shared<MInteger>(lv->Value() - rv->Value());
        } else if (op == "*") {
            return std::make_shared<MInteger>(lv->Value() * rv->Value());
        } else if (op == "/") {
            return std::make_shared<MInteger>(lv->Value() / rv->Value());
        } else if (op == "<") {
            return lv->Value()  < rv->Value() ? M_TRUE : M_FALSE;
        } else if (op == ">") {
            return lv->Value()  > rv->Value() ? M_TRUE : M_FALSE;
        } else if (op == "==") {
            return lv->Value() == rv->Value() ? M_TRUE : M_FALSE;
        } else if (op == "!=") {
            return lv->Value() != rv->Value() ? M_TRUE : M_FALSE;
        }
        return M_NULL;
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
