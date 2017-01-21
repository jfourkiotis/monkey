#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <iostream>
#include <memory>

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

void checkParserErrors(const Parser &p) {
    auto errors = p.Errors();
    if (!errors.empty()) {
        WARN("the parser had " << errors.size() << " errors");
        for(const auto& msg : errors) {
            WARN("parser error: " << msg);
        }
        REQUIRE(errors.empty());
    }
}

TEST_CASE("LetStatement", "[Parsing,LetStatement]") {
    using std::string;
    using namespace token;

    string input = "let x = 5;"
    "let y = 10;"
    "let foobar = 838383;";

    lexer::Lexer l{input};
    Parser p{l};

    auto program = p.ParseProgram();
    checkParserErrors(p);
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

    SECTION("expected let statements") {
        for (int i = 0; i != sizeof(tests)/sizeof(tests[0]); ++i)
        {
            auto stmt = (*program)[i];
            testLetStatement(stmt, tests[i].expected_identifier);
        }
    }
}

TEST_CASE("ReturnStatement", "[Parsing,ReturnStatement]") {
    using std::string;
    
    string input = "return 5;"
    "return 10;"
    "return 993322;";

    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();
    checkParserErrors(p);

    SECTION("expected return statements") {
        REQUIRE(program->size() == 3);

        for (auto i = 0; i != program->size(); ++i) {
            auto stmt = dynamic_cast<ast::ReturnStatement *>((*program)[i]);
            REQUIRE(stmt);
            REQUIRE(stmt->TokenLiteral() == "return");
        }
    }
}

TEST_CASE("ToString", "[Ast]") {
    using std::make_unique;
    using namespace token;
    using namespace ast;
    using std::move;

    Program::Statements ls;
    ls.push_back(
            make_unique<LetStatement>(
                    Token(LET, "let"),
                    make_unique<Identifier>(Token(IDENT, "myVar"), "myVar"),
                    make_unique<Identifier>(Token(IDENT, "anotherVar"), "anotherVar")
            )
    );

    auto program = make_unique<Program>(move(ls));
    REQUIRE(program->ToString() == "let myVar = anotherVar;");
}

TEST_CASE("IdentifierExpression", "[Parsing]") {
    using std::string;

    string input = "foobar;";
    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();
    checkParserErrors(p);

    REQUIRE(program->size() == 1);

    auto stmt = dynamic_cast<ast::ExpressionStatement*>((*program)[0]);
    REQUIRE(stmt);

    auto id = dynamic_cast<const ast::Identifier*>(stmt->BorrowedExpression());
    REQUIRE(id);
    REQUIRE(id->Value() == "foobar");
    REQUIRE(id->TokenLiteral() == "foobar");
}

static void testIntegerLiteral(const ast::Expression *expression, int64_t value) {
    auto literal = dynamic_cast<const ast::IntegerLiteral *>(expression);
    REQUIRE(literal);
    REQUIRE(literal->Value() == value);
    REQUIRE(literal->TokenLiteral() == std::to_string(value));
}

TEST_CASE("IntegerLiteralExpression", "[Parsing]") {
    using std::string;

    string input = "5;";
    lexer::Lexer l{input};
    Parser p{l};

    auto program = p.ParseProgram();
    checkParserErrors(p);

    REQUIRE(program->size() == 1);

    auto stmt = dynamic_cast<ast::ExpressionStatement *>((*program)[0]);
    REQUIRE(stmt);

    auto integer = dynamic_cast<const ast::IntegerLiteral *>(stmt->BorrowedExpression());
    REQUIRE(integer);
    REQUIRE(integer->Value() == 5);
    REQUIRE(integer->TokenLiteral() == "5");
}


TEST_CASE("PrefixExpression", "[Parsing]") {
    using std::string;

    struct {
        string input;
        string op;
        int64_t integer_value;
    } prefixTests[] = {
        {"!5;", "!", 5},
        {"-15;", "-", 15},
    };

    for (const auto& test : prefixTests) {
        lexer::Lexer l{test.input};
        Parser p{l};
        auto program = p.ParseProgram();
        checkParserErrors(p);

        REQUIRE(program->size() == 1);
        auto expressionStmt = dynamic_cast<ast::ExpressionStatement *>((*program)[0]);
        REQUIRE(expressionStmt);
        auto prefixExpression = dynamic_cast<const ast::PrefixExpression *>((expressionStmt->BorrowedExpression()));
        REQUIRE(prefixExpression);
        REQUIRE(prefixExpression->Operator() == test.op);
        testIntegerLiteral(prefixExpression->Right(), test.integer_value);
    }
}

TEST_CASE("InfixExpression", "[Parsing]") {
    using std::string;

    struct {
        string input;
        int64_t left_value;
        string op;
        int64_t right_value;
    } infixTests[] = {
        { "5 + 5;", 5, "+", 5 },
        { "5 - 5;", 5, "-", 5 },
        { "5 * 5;", 5, "*", 5 },
        { "5 / 5;", 5, "/", 5 },
        { "5 > 5;", 5, ">", 5 },
        { "5 < 5;", 5, "<", 5 },
        { "5 == 5;", 5, "==", 5 },
        { "5 != 5;", 5, "!=", 5 },
    };

    for (const auto& test : infixTests) {
        lexer::Lexer l{test.input};
        Parser p{l};
        auto program = p.ParseProgram();
        checkParserErrors(p);

        REQUIRE(program->size() == 1);
        auto expressionStmt = dynamic_cast<ast::ExpressionStatement *>((*program)[0]);
        REQUIRE(expressionStmt);
        auto infixExpression = 
            dynamic_cast<const ast::InfixExpression *>((expressionStmt->BorrowedExpression()));
        REQUIRE(infixExpression);
        testIntegerLiteral(infixExpression->Left(), test.left_value);
        REQUIRE(infixExpression->Operator() == test.op);
        testIntegerLiteral(infixExpression->Right(), test.right_value);
    }
}

