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

const std::unordered_map<token::TokenType, int> precedences = {
    { token::EQ, EQUALS },
    { token::NOT_EQ, EQUALS },
    { token::LT, LESSGREATER },
    { token::GT, LESSGREATER },
    { token::PLUS, SUM },
    { token::MINUS, SUM },
    { token::SLASH, PRODUCT },
    { token::ASTERISK, PRODUCT },
};

using PrefixParseFn = std::function<std::unique_ptr<ast::Expression>()>;
using InfixParseFn = std::function<std::unique_ptr<ast::Expression>(std::unique_ptr<ast::Expression>)>;

class Parser final {
public:
    // take lexer by value (not the best design, but simple enough)
    explicit Parser(lexer::Lexer lexer) : lexer_(lexer) {
        using std::make_unique;
        using namespace token;

        registerPrefix(IDENT, [this] { 
            return make_unique<ast::Identifier>(curToken_, curToken_.literal); 
        });
        registerPrefix(INT, [this] {
            return parseIntegerLiteral();
        });
        registerPrefix(BANG, [this] {
            return parsePrefixExpression();
        });
        registerPrefix(MINUS, [this] {
            return parsePrefixExpression();
        });
        registerInfix(PLUS, [this] (auto left) {
            return parseInfixExpression(std::move(left));
        });
        registerInfix(MINUS, [this] (auto left) {
            return parseInfixExpression(std::move(left));
        });
        registerInfix(SLASH, [this] (auto left) {
            return parseInfixExpression(std::move(left));
        });
        registerInfix(ASTERISK, [this] (auto left) {
            return parseInfixExpression(std::move(left));
        });
        registerInfix(EQ, [this] (auto left) {
            return parseInfixExpression(std::move(left));
        });
        registerInfix(NOT_EQ, [this] (auto left) {
            return parseInfixExpression(std::move(left));
        });
        registerInfix(LT, [this] (auto left) {
            return parseInfixExpression(std::move(left));
        });
        registerInfix(GT, [this] (auto left) {
            return parseInfixExpression(std::move(left));
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

    void noPrefixParseError(token::TokenType t) {
        std::string msg = "no prefix parse function for " + t + " found";
        errors_.push_back(msg);
    }

    int peekPrecedence() const {
        auto p = precedences.find(peekToken_.type);
        return p == precedences.end() ? LOWEST : p->second;
    }

    int curPrecedence() const {
        auto p = precedences.find(curToken_.type);
        return p == precedences.end() ? LOWEST : p->second;
    }

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
            noPrefixParseError(curToken_.type);
            return nullptr;
        }

        auto left = prefix();

        while (!peekTokenIs(token::SEMICOLON) && precedence < peekPrecedence()) {
            auto infix = infixParseFns_.find(peekToken_.type);
            if (infix == infixParseFns_.end()) {
                return left;
            }

            nextToken();

            left = (infix->second)(std::move(left));
        }
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

    std::unique_ptr<ast::Expression> parsePrefixExpression() {
        auto token = curToken_;
        auto op = curToken_.literal;

        nextToken(); // consume ! or -

        auto right = parseExpression(PREFIX);

        return std::make_unique<ast::PrefixExpression>(token, op, std::move(right));
    }

    std::unique_ptr<ast::Expression> parseInfixExpression(std::unique_ptr<ast::Expression> left) {
        auto token = curToken_;
        auto op = curToken_.literal;

        auto precedence = curPrecedence();
        nextToken();
        auto right = parseExpression(precedence);

        return std::make_unique<ast::InfixExpression>(token, std::move(left), op, std::move(right));
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

