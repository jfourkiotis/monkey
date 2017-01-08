#ifndef MONKEY_PARSER_H_INCLUDED
#define MONKEY_PARSER_H_INCLUDED

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <sstream>
#include <cstdint>

#include "token.h"
#include "lexer.h"
#include "ast.h"

enum Precedence {
    LOWEST,
    EQUALS,         // ==
    LESSGREATER,    // > or <
    SUM,            // +
    PRODUCT,        // *
    PREFIX,         // -X OR !X
    CALL,           // myFunction(X)
};


using PrefixParseFn = std::function<std::unique_ptr<ast::Expression>()>;
using InfixParseFn = std::function<std::unique_ptr<ast::Expression>(std::unique_ptr<ast::Expression>)>;

class Parser final {
public:
    // take lexer by value (not the best design, but simple enough)
    explicit Parser(lexer::Lexer lexer) : lexer_(lexer) {
        using std::make_unique;
        using namespace token;

        registerPrefix(IDENT, [this]{ 
            return make_unique<ast::Identifier>(curToken_, curToken_.literal); 
        });
        registerPrefix(INT, [this]{
            return parseIntegerLiteral();
        });
    
    }

    std::unique_ptr<ast::Program> ParseProgram() {
        using std::make_unique;
        using std::move;

        ast::Program::Statements statements;

        while (curToken_.type != token::EOF_) {
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(move(stmt));
            }
            nextToken();
        }

        return make_unique<ast::Program>(move(statements));
    }

    using ErrorList = std::vector<std::string>;
    ErrorList Errors() const { return errors_; }
private:

    void peekError(token::TokenType t) {
        auto msg = "expected next token to be " + t + ". got " + peekToken_.type + " instead";
        errors_.push_back(msg);
    }

    void nextToken() {
        curToken_ = peekToken_;
        peekToken_ = lexer_.NextToken();
    }

    std::unique_ptr<ast::Statement> parseStatement() {
        if (curToken_.type == token::LET) {
            return parseLetStatement();
        } else if (curToken_.type == token::RETURN) {
            return parseReturnStatement();
        } else {
            return parseExpressionStatement();
        }
    }

    std::unique_ptr<ast::Statement> parseLetStatement() {
        using std::make_unique;
        using std::move;

        auto let_token = curToken_;
        if (!expectPeek(token::IDENT)) {
            return nullptr;
        }

        auto identifier = make_unique<ast::Identifier>(curToken_, curToken_.literal);

        if (!expectPeek(token::ASSIGN)) {
            return nullptr;
        }

        // TODO: we're skipping the expressions until we encounter
        // a semicolon
        while (!curTokenIs(token::SEMICOLON)) {
            nextToken();
        }

        return make_unique<ast::LetStatement>(let_token, move(identifier), nullptr);
    }

    std::unique_ptr<ast::Statement> parseReturnStatement() {
        auto return_token = curToken_;
        nextToken();

        // TODO: we're skipping the expressions until we encounter
        // a semicolon
        while (!curTokenIs(token::SEMICOLON)) {
            nextToken();
        }

        return std::make_unique<ast::ReturnStatement>(return_token, nullptr);
    }

    std::unique_ptr<ast::ExpressionStatement> parseExpressionStatement() {
        auto current = curToken_;
        auto expression = parseExpression(LOWEST);

        if (peekTokenIs(token::SEMICOLON)) {
            nextToken();
        }

        return std::make_unique<ast::ExpressionStatement>(current, std::move(expression));
    }

    std::unique_ptr<ast::Expression> parseExpression(int precedence) {
        auto prefix = prefixParseFns_[curToken_.type];
        if (!prefix) {
            return nullptr;
        }

        auto left = prefix();
        return left;
    }

    std::unique_ptr<ast::Expression> parseIntegerLiteral() {
        std::istringstream stream{curToken_.literal};
        int64_t value{0};
        stream >> value;

        if (!stream) {
            std::string msg = "could not parse " + curToken_.literal + "as integer";
            errors_.push_back(msg);
            return nullptr;
        }
        return std::make_unique<ast::IntegerLiteral>(curToken_, value);
    }

    bool curTokenIs(token::TokenType type) const {
        return curToken_.type == type;
    }

    bool peekTokenIs(token::TokenType type) const {
        return peekToken_.type == type;
    }

    bool expectPeek(token::TokenType type) {
        if (peekTokenIs(type)) {
            nextToken();
            return true;
        } else {
            peekError(type);
            return false;
        }
    }

    void registerPrefix(token::TokenType type, const PrefixParseFn &fn) {
        prefixParseFns_[type] = fn;
    }

    void registerInfix(token::TokenType type, const InfixParseFn &fn) {
        infixParseFns_[type] = fn;
    }

    lexer::Lexer lexer_;
    token::Token curToken_ = lexer_.NextToken();
    token::Token peekToken_ = lexer_.NextToken();
    ErrorList errors_;

    std::unordered_map<token::TokenType, PrefixParseFn> prefixParseFns_;
    std::unordered_map<token::TokenType, InfixParseFn > infixParseFns_ ;
};//~ Parser

#endif // MONKEY_PARSER_H_INCLUDED

