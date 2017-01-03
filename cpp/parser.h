#ifndef MONKEY_PARSER_H_INCLUDED
#define MONKEY_PARSER_H_INCLUDED

#include <memory>
#include <vector>
#include <string>
#include "token.h"
#include "lexer.h"
#include "ast.h"


class Parser final {
public:
    // take lexer by value (not the best design, but simple enough)
    explicit Parser(lexer::Lexer lexer) : lexer_(lexer) {}

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
        errors_.emplace_back(msg);
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
        }
        return nullptr;
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

    lexer::Lexer lexer_;
    token::Token curToken_ = lexer_.NextToken();
    token::Token peekToken_ = lexer_.NextToken();
    ErrorList errors_;
};//~ Parser

#endif // MONKEY_PARSER_H_INCLUDED

