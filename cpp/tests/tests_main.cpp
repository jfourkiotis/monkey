#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <iostream>
#include <memory>

#include "../token.h"
#include "../lexer.h"
#include "../ast.h"
#include "../parser.h"
#include "../object.h"
#include "../eval.h"

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
        {"x"},
        {"y"},
        {"foobar"},
    };

    auto testLetStatement = [](auto stmt, auto str) -> bool {
        REQUIRE(stmt->TokenLiteral() == "let");

        auto letStmt = dynamic_cast<const ast::LetStatement *>(stmt);
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
            auto stmt = dynamic_cast<const ast::ReturnStatement *>((*program)[i]);
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

    Program::StatementList ls;
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

    auto stmt = dynamic_cast<const ast::ExpressionStatement*>((*program)[0]);
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

static void testIdentifier(const ast::Expression *expression, const std::string& str) {
    auto ident = dynamic_cast<const ast::Identifier *>(expression);
    REQUIRE(ident);
    REQUIRE(ident->Value() == str);
    REQUIRE(ident->TokenLiteral() == str);
}

static void testBooleanLiteral(const ast::Expression *expression, bool v) {
    auto literal = dynamic_cast<const ast::BooleanLiteral *>(expression);
    REQUIRE(literal);
    REQUIRE(literal->Value() == v);
    REQUIRE(literal->TokenLiteral() == (v ? "true" : "false"));
}

static void testLiteralExpression(const ast::Expression *expression, int64_t v) {
    testIntegerLiteral(expression, v);
}

static void testLiteralExpression(const ast::Expression *expression, const std::string& s) {
    testIdentifier(expression, s);
}

static void testLiteralExpression(const ast::Expression *expression, bool b) {
    testBooleanLiteral(expression, b);
}

static void testInfixExpression(const ast::Expression *expression, int64_t l, const std::string& op, int64_t r) {
    auto infix = dynamic_cast<const ast::InfixExpression *>(expression);
    REQUIRE(infix);
    testLiteralExpression(infix->Left(), l);
    REQUIRE(infix->Operator() == op);
    testLiteralExpression(infix->Right(), r);
}

static void testInfixExpression(const ast::Expression *expression, const std::string& l, const std::string& op, const std::string& r) {
    auto infix = dynamic_cast<const ast::InfixExpression *>(expression);
    REQUIRE(infix);
    testLiteralExpression(infix->Left(), l);
    REQUIRE(infix->Operator() == op);
    testLiteralExpression(infix->Right(), r);
}

TEST_CASE("IntegerLiteralExpression", "[Parsing]") {
    using std::string;

    string input = "5;";
    lexer::Lexer l{input};
    Parser p{l};

    auto program = p.ParseProgram();
    checkParserErrors(p);

    REQUIRE(program->size() == 1);

    auto stmt = dynamic_cast<const ast::ExpressionStatement *>((*program)[0]);
    REQUIRE(stmt);

    auto integer = dynamic_cast<const ast::IntegerLiteral *>(stmt->BorrowedExpression());
    REQUIRE(integer);
    REQUIRE(integer->Value() == 5);
    REQUIRE(integer->TokenLiteral() == "5");
}

template<typename T>
void TestPrefixCore(const std::string& input, const std::string& op, T val)
{
    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();
    checkParserErrors(p);

    REQUIRE(program->size() == 1);
    auto expressionStmt = 
        dynamic_cast<const ast::ExpressionStatement *>((*program)[0]);
    REQUIRE(expressionStmt);
    auto prefixExpression = 
        dynamic_cast<const ast::PrefixExpression *>((expressionStmt->BorrowedExpression()));
    REQUIRE(prefixExpression);
    REQUIRE(prefixExpression->Operator() == op);
    testLiteralExpression(prefixExpression->Right(), val);
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
        TestPrefixCore(test.input, test.op, test.integer_value);
    }

    struct {
        string input;
        string op;
        bool value;
    } prefixBooleanTests[] = {
        {"!true;", "!", true},
        {"!false", "!", false},
    };

    for (const auto& test : prefixBooleanTests) {
        TestPrefixCore(test.input, test.op, test.value);
    }
}

template<typename T>
void TestInfixCore(const std::string& input, T left, const std::string &op, T right)
{
    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();
    checkParserErrors(p);

    REQUIRE(program->size() == 1);
    auto expressionStmt = dynamic_cast<const ast::ExpressionStatement *>((*program)[0]);
    REQUIRE(expressionStmt);
    auto infixExpression =
        dynamic_cast<const ast::InfixExpression *>((expressionStmt->BorrowedExpression()));
    REQUIRE(infixExpression);
    testLiteralExpression(infixExpression->Left(), left);
    REQUIRE(infixExpression->Operator() == op);
    testLiteralExpression(infixExpression->Right(), right);
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
        TestInfixCore(test.input, test.left_value, test.op, test.right_value);
    }

    struct {
        string input;
        bool left_value;
        string op;
        bool right_value;
    } infixBooleanTests[] = {
        { "true == true", true, "==", true },
        { "true != false", true, "!=", false },
        { "false == false", false, "==", false },
    };

    for (const auto& test : infixBooleanTests) {
        TestInfixCore(test.input, test.left_value, test.op, test.right_value);
    }
}

TEST_CASE("OperatorPrecedence", "[Parsing]") {
    using std::string;

    struct {
        std::string input;
        std::string expected;
    } tests[] = {
        { "-a * b", "((-a) * b)" },
        { "a + b + c", "((a + b) + c)" },
        { "a + b - c", "((a + b) - c)" },
        { "a * b * c", "((a * b) * c)" },
        { "a * b / c", "((a * b) / c)" },
        { "a + b / c", "(a + (b / c))" },
        { "a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)" },
        { "3 + 4; -5 * 5", "(3 + 4)((-5) * 5)" },
        { "5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))" },
        { "3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))" },
        { "3 + 4 * 5 != 3 * 1 + 4 * 5", "((3 + (4 * 5)) != ((3 * 1) + (4 * 5)))" },
        { "true", "true" },
        { "false", "false" },
        { "3 > 5 == false", "((3 > 5) == false)" },
        { "3 < 5 == true" , "((3 < 5) == true)"  },
        { "1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)" },
        { "(5 + 5) * 2", "((5 + 5) * 2)" },
        { "2 / (5 + 5)", "(2 / (5 + 5))" },
        { "-(5 + 5)", "(-(5 + 5))" },
        { "!(true == true)", "(!(true == true))" },
        { "a + add(b * c) + d", "((a + add((b * c))) + d)" },
        { "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))", "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
        { "add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))" },
    };

    for(const auto& test : tests) {
        lexer::Lexer l{test.input};
        Parser p{l};
        auto program = p.ParseProgram();
        checkParserErrors(p);

        auto actual = program->ToString();
        REQUIRE(actual == test.expected);
    }
}

TEST_CASE("IfExpression", "[Parsing]") {
    using std::string;

    std::string input{"if (x < y) { x } else { y }"};
    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();
    checkParserErrors(p);

    REQUIRE(program);
    REQUIRE(program->size() == 1);
    auto expressionStmt = 
        dynamic_cast<const ast::ExpressionStatement *>((*program)[0]);
    REQUIRE(expressionStmt);
    auto ifExpression = 
        dynamic_cast<const ast::IfExpression *>(expressionStmt->BorrowedExpression());
    REQUIRE(ifExpression);
    testInfixExpression(ifExpression->Condition(), "x", "<", "y");
    auto consequence_block = ifExpression->Consequence();
    REQUIRE(consequence_block->size() == 1);

    auto expressionStmt2 = 
        dynamic_cast<const ast::ExpressionStatement *>((*consequence_block)[0]);
    REQUIRE(expressionStmt2);
    testIdentifier(expressionStmt2->BorrowedExpression(), "x");

    auto alternative_block = ifExpression->Alternative();
    REQUIRE(alternative_block->size() == 1);
    auto expressionStmt3 = 
        dynamic_cast<const ast::ExpressionStatement *>((*alternative_block)[0]);
    REQUIRE(expressionStmt3);
    testIdentifier(expressionStmt3->BorrowedExpression(), "y");

}

TEST_CASE("FunctionParameters", "[Parsing]") {
    using std::string;
    using std::vector;

    struct {
        string input;
        vector<string> expectedParams;
    } tests[] = {
        { "fn() {};", {} },
        { "fn(x) {};", {"x"} },
        { "fn(x, y, z) {};", {"x", "y", "z"}}
    };

    for (const auto& tt : tests) {
        lexer::Lexer l{tt.input};
        Parser p{l};
        auto program = p.ParseProgram();
        checkParserErrors(p);

        auto stmt = 
            dynamic_cast<const ast::ExpressionStatement *>((*program)[0]);
        REQUIRE(stmt);
        auto function = 
            dynamic_cast<const ast::FunctionLiteral *>(stmt->BorrowedExpression());
        REQUIRE(function);
        REQUIRE(function->Parameters().size() == tt.expectedParams.size());

        int i = 0;
        for (const auto &ep : tt.expectedParams) {
            testLiteralExpression(function->Parameters()[i].get(), ep);
            ++i;
        }
    }
}

TEST_CASE("FunctionLiteral", "[Parsing]") {
    using std::string;

    std::string input{"fn(x,y) {x + y;}"};
    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();

    REQUIRE(program);
    REQUIRE(program->size() == 1);
    auto expressionStmt = 
        dynamic_cast<const ast::ExpressionStatement *>((*program)[0]);
    REQUIRE(expressionStmt);
    auto function = 
        dynamic_cast<const ast::FunctionLiteral *>(expressionStmt->BorrowedExpression());

    const auto& params = function->Parameters();
    REQUIRE(params.size() == 2);

    testLiteralExpression(params[0].get(), string("x"));
    testLiteralExpression(params[1].get(), string("y"));

    auto body = function->Body();
    REQUIRE(body->size() == 1);
    auto bodyStmt = dynamic_cast<const ast::ExpressionStatement *>((*body)[0]);
    REQUIRE(bodyStmt);
    testInfixExpression(bodyStmt->BorrowedExpression(), "x", "+", "y");
}

TEST_CASE("CallExpression", "[Parsing]") {
    using std::string;

    std::string input{"add(1, 2 * 3, 4 + 5);"};

    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();
    checkParserErrors(p);

    REQUIRE(program);
    REQUIRE(program->size() == 1);
    auto expressionStmt = 
        dynamic_cast<const ast::ExpressionStatement *>((*program)[0]);
    REQUIRE(expressionStmt);
    auto callExpression = 
        dynamic_cast<const ast::CallExpression *>(expressionStmt->BorrowedExpression());
    REQUIRE(callExpression);
    testIdentifier(callExpression->Function(), "add");

    const auto& args = callExpression->Arguments();
    REQUIRE(args.size() == 3);
    testLiteralExpression(args[0].get(), static_cast<int64_t>(1));
    testInfixExpression(args[1].get(), 2, "*", 3);
    testInfixExpression(args[2].get(), 4, "+", 5);
}

static std::shared_ptr<MObject> testEval(const std::string& input) {
    lexer::Lexer l{input};
    Parser p{l};
    auto program = p.ParseProgram();
    REQUIRE(program);
    checkParserErrors(p);
    return Eval(*program);
}

template<typename MType, typename RType>
static void testLiteralObject(const std::shared_ptr<MObject>& obj, RType expected) {
    auto result = std::static_pointer_cast<MType>(obj);
    REQUIRE(result != nullptr);
    REQUIRE(result->Value() == expected);
}

TEST_CASE("EvalIntegerExpression", "[Evaluator]") {
    struct {
        std::string input;
        int64_t expected;
    } tests[] = {
        { "5",  5},
        {"10", 10},
        {"-5", -5},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2 ", 32},
        {"-50 + 100 + -50", 0},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"20 + 2 * -10", 0},
        {"50 / 2 * 2 + 10", 60},
        {"2 * (5 + 10)", 30},
        {"3 * 3 * 3 + 10", 37},
        {"3 * (3 * 3) + 10", 37},
        {"(5 + 10 * 2 + 15/ 3) * 2 + -10", 50},
    };

    for (const auto& tt : tests) {
        auto evaluated = testEval(tt.input);
        testLiteralObject<MInteger>(evaluated, tt.expected);
    }
}

TEST_CASE("EvalBangOperator", "[Evaluator]") {
    struct {
        std::string input;
        bool expected;
    } tests[] = {
        {"!true"  , false},
        {"!false" , true },
        {"!5"     , false},
        {"!!true" , true },
        {"!!false", false},
        {"!!5"    , true }
    };
    
    for (const auto& tt : tests) {
        auto evaluated = testEval(tt.input);
        testLiteralObject<MBoolean>(evaluated, tt.expected);
    }
}
