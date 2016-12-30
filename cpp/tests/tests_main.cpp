#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <iostream>

#include "../token.h"
#include "../lexer.h"
#include "../ast.h"
#include "../parser.h"

// Lexer tests
TEST_CASE("lexer", "[lexer]") {
    using std::string;
    using namespace token;

    auto input = "let five = 5;"
    "let ten = 10;"
    ""
    "let add = fn(x, y) {"
    "  x + y;"
    "};"
    ""
    "let result = add(five, ten);"
    "!-/*5;"
    "5 < 10 > 5;"
    ""
    "if (5 < 10) {"
    "  return true;"
    "} else {"
    "  return false;"
    "}"
    ""
    "10 == 10;"
    "10 != 9;"
    "";

    struct {
        TokenType expected_type;
        string expected_literal;
    } tests[] = {
        {LET, "let"},
        {IDENT, "five"},
        {ASSIGN, "="},
        {INT, "5"},
        {SEMICOLON, ";"},
        {LET, "let"},
        {IDENT, "ten"},
        {ASSIGN, "="},
        {INT, "10"},
        {SEMICOLON, ";"},
        {LET, "let"},
        {IDENT, "add"},
        {ASSIGN, "="},
        {FUNCTION, "fn"},
        {LPAREN, "("},
        {IDENT, "x"},
        {COMMA, ","},
        {IDENT, "y"},
        {RPAREN, ")"},
        {LBRACE, "{"},
        {IDENT, "x"},
        {PLUS, "+"},
        {IDENT, "y"},
        {SEMICOLON, ";"},
        {RBRACE, "}"},
        {SEMICOLON, ";"},
        {LET, "let"},
        {IDENT, "result"},
        {ASSIGN, "="},
        {IDENT, "add"},
        {LPAREN, "("},
        {IDENT, "five"},
        {COMMA, ","},
        {IDENT, "ten"},
        {RPAREN, ")"},
        {SEMICOLON, ";"},
        {BANG, "!"},
        {MINUS, "-"},
        {SLASH, "/"},
        {ASTERISK, "*"},
        {INT, "5"},
        {SEMICOLON, ";"},
        {INT, "5"},
        {LT, "<"},
        {INT, "10"},
        {GT, ">"},
        {INT, "5"},
        {SEMICOLON, ";"},
        {IF, "if"},
        {LPAREN, "("},
        {INT, "5"},
        {LT, "<"},
        {INT, "10"},
        {RPAREN, ")"},
        {LBRACE, "{"},
        {RETURN, "return"},
        {TRUE_, "true"},
        {SEMICOLON, ";"},
        {RBRACE, "}"},
        {ELSE, "else"},
        {LBRACE, "{"},
        {RETURN, "return"},
        {FALSE_, "false"},
        {SEMICOLON, ";"},
        {RBRACE, "}"},
        {INT, "10"},
        {EQ, "=="},
        {INT, "10"},
        {SEMICOLON, ";"},
        {INT, "10"},
        {NOT_EQ, "!="},
        {INT, "9"},
        {SEMICOLON, ";"},
        {EOF_   , ""},
    };

    lexer::Lexer l(input);

    SECTION("expected token type & literal") {
        for (int i = 0; i != sizeof(tests)/sizeof(tests[0]); ++i)
        {
            auto tok = l.NextToken();
            REQUIRE(tok.type == tests[i].expected_type);
            REQUIRE(tok.literal == tests[i].expected_literal);
        }
    }
}

TEST_CASE("parser", "[LetStatement]") {
    using std::string;
    using namespace token;

    string input = "let x = 5;"
    "let y = 10;"
    "let foobar = 838383;";

    lexer::Lexer l{input};
    Parser p{l};

    auto program = p.ParseProgram();
    REQUIRE(program);
    REQUIRE(program->size() == 3);

    struct {
        string expected_identifier;
    } tests[] = {
        "x",
        "y",
        "foobar",
    };

    auto testLetStatement = [](auto stmt, auto str) -> bool {
        REQUIRE(stmt->TokenLiteral() == "let");

        auto *letStmt = dynamic_cast<ast::LetStatement *>(stmt);
        REQUIRE(letStmt != nullptr);
        REQUIRE(letStmt->Name()->Value() == str);
        REQUIRE(letStmt->Name()->TokenLiteral() == str);

        return true;
    };

    SECTION("expected let statement") {
        for (int i = 0; i != sizeof(tests)/sizeof(tests[0]); ++i)
        {
            auto stmt = (*program)[i];
            testLetStatement(stmt, tests[i].expected_identifier);
        }
    }
}

