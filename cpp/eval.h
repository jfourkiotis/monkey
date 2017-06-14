#ifndef MONKEY_EVAL_H_INCLUDED
#define MONKEY_EVAL_H_INCLUDED

#include <memory>
#include <vector>
#include <cassert>

#include "ast.h"
#include "ast_visitor.h"
#include "object.h"
#include "env.h"

std::shared_ptr<MObject> Eval(const ast::Node& node, Environment *env);

namespace {

using namespace ast;
    
std::shared_ptr<MBoolean> M_TRUE = std::make_shared<MBoolean>(true);
std::shared_ptr<MBoolean> M_FALSE= std::make_shared<MBoolean>(false);
std::shared_ptr<MNull> M_NULL = std::make_shared<MNull>();

class EvalVisitor final : public AstVisitor {
public:

    explicit EvalVisitor(Environment* env) : env_(env) {}
    
    void Visit(const Program &node) override {
        _EvalProgram(node);
    };

    void Visit(const Identifier& node) override {
        result.push_back(_EvalIdentifier(node, env_));
    };

    void Visit(const LetStatement& node) override {
        auto val = Eval(*node.Value(), env_);
        if (_IsError(val)) {
            result.push_back(val);
        } else {
            env_->Set(node.Name()->Value(), val);
            result.push_back(nullptr);
        }
    }

    void Visit(const ReturnStatement& node) override {
        auto tmp = Eval(*node.Value(), env_);
        if (_IsError(tmp)) {
            result.push_back(tmp);
        } else {
            result.push_back(std::make_shared<MReturn>(tmp));
        }
    }

    void Visit(const ExpressionStatement& node) override {
        node.BorrowedExpression()->AcceptVisitor(*this);
    }

    void Visit(const PrefixExpression& node) override {
        auto tmp = Eval(*node.Right(), env_);
        if (_IsError(tmp)) {
            result.push_back(tmp);
        } else {
            result.push_back(_EvalPrefixExpression(node.Operator(), tmp));
        }
    }

    void Visit(const InfixExpression& node) override {
        auto left = Eval(*node.Left(), env_);
        if (_IsError(left)) {
            result.push_back(left);
            return;
        }
        auto right= Eval(*node.Right(), env_);
        if (_IsError(right)) {
            result.push_back(right);
            return;
        }
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
        _EvalBlockStatement(node);
    }

    void Visit(const IfExpression& node) override {
        auto condition = Eval(*node.Condition(), env_);
        if (_IsError(condition)) {
            result.push_back(condition);
            return;
        }
        if (_IsTruthy(condition)) {
            result.push_back(Eval(*node.Consequence(), env_));
        } else if (node.Alternative()) {
            result.push_back(Eval(*node.Alternative(), env_));
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
    
    void _EvalProgram(const Program& program) {
        std::shared_ptr<MObject> final;
        for (const auto& stmt : program.Statements()) {
            final = Eval(*stmt, env_);
            
            if (!final) continue;
            
            if (final->Type() == ObjectType::RETURN_VALUE_OBJ) {
                auto r = std::static_pointer_cast<MReturn>(final);
                result.push_back(r->Value());
                return;
            } else if (final->Type() == ObjectType::ERROR_OBJ) {
                break; // error is returned
            }
        }
        result.push_back(final);
    }
    
    void _EvalBlockStatement(const BlockStatement& block) {
        std::shared_ptr<MObject> final;
        for (const auto& stmt : block.Statements()) {
            final = Eval(*stmt, env_);
            if (final && (final->Type() == ObjectType::RETURN_VALUE_OBJ || final->Type() == ObjectType::ERROR_OBJ)) {
                result.push_back(final);
                return;
            }
        }
        result.push_back(final);
    }
    
    std::shared_ptr<MObject> _EvalPrefixExpression(const std::string& op, const std::shared_ptr<MObject>& obj) {
        if (op == "!") {
            return _EvalBangOperatorExpression(obj);
        } else if (op == "-") {
            return _EvalMinusPrefixOperatorExpression(obj);
        }
        return _NewError("unknown operator: %s%s", op.c_str(), GetObjectTypeName(obj->Type()));
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
            return _NewError("unknown operator: -%s",
                             GetObjectTypeName(obj->Type()));
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
        } else if (left->Type() != right->Type()) {
            return _NewError("type mismatch: %s %s %s",
                             GetObjectTypeName(left->Type()),
                             op.c_str(),
                             GetObjectTypeName(right->Type()));
        } else {
            return _NewError("unknown operator: %s %s %s",
                             GetObjectTypeName(left->Type()),
                             op.c_str(),
                             GetObjectTypeName(right->Type()));
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
        return _NewError("unknown operator: %s %s %s",
                         GetObjectTypeName(left->Type()),
                         op.c_str(),
                         GetObjectTypeName(right->Type()));
    }
    
    std::shared_ptr<MObject> _EvalIdentifier(const Identifier& node, Environment *env)
    {
        auto r = env->Get(node.Value());
        if (r == nullptr) {
            return _NewError("identifier not found: %s", node.Value().c_str());
        }
        return r;
    }
    
    template<typename ...Args>
    std::shared_ptr<MError> _NewError(const std::string& format, Args ...args) {
        auto size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
        auto bufr = std::make_unique<char[]>(size);
        snprintf(bufr.get(), size, format.c_str(), args...);
        return std::make_shared<MError>(std::string(bufr.get(), bufr.get() + size - 1));
    }
    
    bool _IsError(const std::shared_ptr<MObject>& obj) const {
        return !obj || obj->Type() == ObjectType::ERROR_OBJ;
    }
    
    std::vector<std::shared_ptr<MObject>> result;
    Environment* env_;
};//~ EvalVisitor

}

std::shared_ptr<MObject> Eval(const ast::Node& node, Environment* env) {
    EvalVisitor visitor(env);
    node.AcceptVisitor(visitor);
    return visitor.Result();
}

#endif
