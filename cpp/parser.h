#ifndef MONKEY_PARSER_H_INCLUDED
#define MONKEY_PARSER_H_INCLUDED

#include <memory>
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
private:

    void nextToken() {
        curToken_ = peekToken_;
        peekToken_ = lexer_.NextToken();
    }

    std::unique_ptr<ast::Statement> parseStatement() {
        if (curToken_.type == token::LET) {
            return parseLetStatement();
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
            return false;
        }
    }

    lexer::Lexer lexer_;
    token::Token curToken_ = lexer_.NextToken();
    token::Token peekToken_ = lexer_.NextToken();
};//~ Parser

#endif // MONKEY_PARSER_H_INCLUDED

