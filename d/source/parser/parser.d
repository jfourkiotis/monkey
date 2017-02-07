module parser;

import token;
import lexer;
import ast;

alias PrefixParseFn = Expression delegate();
alias InfixParseFn = Expression delegate(Expression);

enum Precedence {
    LOWEST,
    EQUALS,
    LESSGREATER,
    SUM,
    PRODUCT,
    PREFIX,
    CALL
}

struct Parser {
private:
    static immutable(int[TokenType]) precedences;

    static this() {
        precedences[EQ] = Precedence.EQUALS;    
        precedences[NOT_EQ] = Precedence.EQUALS;    
        precedences[LT] = Precedence.LESSGREATER;    
        precedences[GT] = Precedence.LESSGREATER;    
        precedences[PLUS] = Precedence.SUM;    
        precedences[MINUS] = Precedence.SUM;    
        precedences[SLASH] = Precedence.PRODUCT;    
        precedences[ASTERISK] = Precedence.PRODUCT;    
    }

    Lexer lexer_;
    Token curToken_;
    Token peekToken_;
    string[] errors_;
    PrefixParseFn[TokenType] prefixParseFns_;
    InfixParseFn[TokenType] infixParseFns_;

    void registerPrefix(TokenType type, PrefixParseFn fn) {
        prefixParseFns_[type] = fn;
    }

    void registerInfix(TokenType type, InfixParseFn fn) { 
        infixParseFns_[type] = fn;
    }

    int curPrecedence() const {
        auto prec = (curToken_.type in precedences);
        if (prec !is null) {
            return *prec;
        }
        return Precedence.LOWEST;
    }


    int peekPrecedence() const {
        auto prec = (peekToken_.type in precedences);
        if (prec !is null) {
            return *prec;
        }
        return Precedence.LOWEST;
    }

    void peekError(TokenType t) {
        import std.string;
        string msg = format("expected next token to be %s, got %s instead", t, peekToken_.type);
        errors_ ~= msg;
    }

    void nextToken() {
        curToken_ = peekToken_;
        peekToken_ = lexer_.nextToken();
    }

    Statement parseStatement() {
        switch (curToken_.type) {
        case LET:
            return parseLetStatement();
        case RETURN:
            return parseReturnStatement();
        default:
            return parseExpressionStatement();
        }
    }

    LetStatement parseLetStatement() {
        auto letToken = curToken_;
        if (!expectPeek(IDENT)) {
            return null;
        }

        auto name = new Identifier(curToken_, curToken_.literal);

        if (!expectPeek(ASSIGN)) {
            return null;
        }

        // TODO: We're skipping the expressions until we
        // encounter a semicolon
        while (!curTokenIs(SEMICOLON)) {
            nextToken();
        }

        return new LetStatement(letToken, name, null);
    }

    ReturnStatement parseReturnStatement() {
        auto retToken = curToken_;

        nextToken();

        while(!curTokenIs(SEMICOLON)) {
            nextToken();
        }

        return new ReturnStatement(retToken, null);
    }

    ExpressionStatement parseExpressionStatement() {
        auto exprToken = curToken_;
        auto expression = parseExpression(Precedence.LOWEST);

        if (peekTokenIs(SEMICOLON)) {
            nextToken();
        }

        return new ExpressionStatement(exprToken, expression);
    }

    void noPrefixParseFnError(TokenType type) {
        import std.string : format;
        string msg = format("no prefix parse function for '%s' found", type);
        errors_ ~= msg;
    }

    Expression parsePrefixExpression() {
        auto current = curToken_;
        auto operator = curToken_.literal;

        nextToken(); // consume operator
        auto right = parseExpression(Precedence.PREFIX);
        return new PrefixExpression(current, operator, right);
    }

    Expression parseExpression(int precedence) {
        auto prefix = (curToken_.type in prefixParseFns_);
        if (prefix is null) {
            noPrefixParseFnError(curToken_.type);
            return null;
        }

        auto left = (*prefix)();
        while (!peekTokenIs(SEMICOLON) && precedence < peekPrecedence()) {
            auto infix = (peekToken_.type in infixParseFns_);
            if (infix is null) {
                return left;
            }

            nextToken();
            left = (*infix)(left);
        }

        return left;
    }

    Expression parseInfixExpression(Expression left) {
        auto current = curToken_;
        auto operator = curToken_.literal;
        auto precedence = curPrecedence();

        nextToken();
        auto right = parseExpression(precedence);
        return new InfixExpression(current, left, operator, right);
    }

    Expression parseGroupedExpression() {
        nextToken();
        auto exp = parseExpression(Precedence.LOWEST);
        if (!expectPeek(RPAREN)) {
            return null;
        }
        return exp;
    }

    Expression parseIfExpression() {
        auto current = curToken_; // if
        
        if (!expectPeek(LPAREN)) {
            return null;
        }

        nextToken();
        auto condition = parseExpression(Precedence.LOWEST);

        if (!expectPeek(RPAREN)) {
            return null;
        }

        if (!expectPeek(LBRACE)) {
            return null;
        }

        auto consequence = parseBlockStatement();

        BlockStatement alternative = null;
        if (peekTokenIs(ELSE)) {
            nextToken();

            if (!expectPeek(LBRACE)) {
                return null;
            }

            alternative = parseBlockStatement();
        }   

        return new IfExpression(current, condition, consequence, alternative);
    }

    BlockStatement parseBlockStatement() {
        auto current = curToken_;
        Statement[] statements;

        nextToken();

        while (!curTokenIs(RBRACE)) {
            auto stmt = parseStatement();
            if (stmt !is null) {
                statements ~= stmt;
            }
            nextToken();
        }

        return new BlockStatement(current, statements);
    }

    bool curTokenIs(TokenType t) const {
        return curToken_.type == t;
    }

    bool peekTokenIs(TokenType t) const {
        return peekToken_.type == t;
    }

    bool expectPeek(TokenType t) {
        if (peekTokenIs(t)) {
            nextToken();
            return true;
        } else {
            peekError(t);
            return false;
        }
    }

    Expression parseIdentifier() {
        return new Identifier(curToken_, curToken_.literal);
    }

    Expression parseIntegerLiteral() {
        import std.conv;
    
        try {
            long val = to!long(curToken_.literal);
            return new IntegerLiteral(curToken_, val);
        } catch (Exception e) {
            errors_ ~= e.msg;
        }
        return null;
    }

    Expression parseBooleanLiteral() {
        return new BooleanLiteral(curToken_, curTokenIs(token.TRUE));
    }

public:
    @disable this();

    this(Lexer lexer) {
        lexer_ = lexer;
        nextToken();
        nextToken();

        registerPrefix(IDENT, delegate() { return new Identifier(curToken_, curToken_.literal); });
        registerPrefix(INT, delegate() { return parseIntegerLiteral(); });
        registerPrefix(BANG, delegate() { return parsePrefixExpression(); });
        registerPrefix(MINUS, delegate() { return parsePrefixExpression(); });
        registerPrefix(TRUE, delegate() { return parseBooleanLiteral(); });
        registerPrefix(FALSE, delegate() { return parseBooleanLiteral(); });
        registerPrefix(LPAREN, delegate() { return parseGroupedExpression(); });
        registerPrefix(IF, delegate() { return parseIfExpression(); });
        //
        registerInfix(PLUS, delegate(Expression left) { return parseInfixExpression(left); });
        registerInfix(MINUS, delegate(Expression left) { return parseInfixExpression(left); });
        registerInfix(SLASH, delegate(Expression left) { return parseInfixExpression(left); });
        registerInfix(ASTERISK, delegate(Expression left) { return parseInfixExpression(left); });
        registerInfix(EQ, delegate(Expression left) { return parseInfixExpression(left); });
        registerInfix(NOT_EQ, delegate(Expression left) { return parseInfixExpression(left); });
        registerInfix(LT, delegate(Expression left) { return parseInfixExpression(left); });
        registerInfix(GT, delegate(Expression left) { return parseInfixExpression(left); });
    }

    string[] errors() { return errors_.dup; }

    Program parseProgram() {
        Statement[] statements;

        while (curToken_.type != EOF) {
            auto stmt = parseStatement();
            if (stmt) {
                statements ~= stmt;
            }
            nextToken();
        }

        return new Program(statements);
    }
}

unittest {

    import std.stdio : writeln, writefln;
    import std.string : format;

    void checkParserErrors(Parser p) {
        auto errors = p.errors();
        if (errors.length)
        {
            writefln("the parser has %d errors", errors.length);
            foreach(msg; errors) {
                writefln("parser error: %s", msg);
            }
            assert(errors.length == 0, "the parser encountered errors");
        }
    }

    {
        string input = "let x = 5;" ~
            "let y = 10;" ~
            "let foobar = 838383;";

        auto l = Lexer(input);
        auto p = Parser(l);
        auto program = p.parseProgram();

        checkParserErrors(p);

        assert(program !is null, "parseProgram() returned null");
        assert(program.length == 3, "program.statements does not contain 3 statements");

        struct T{ string expectedIdentifier; } 
        T[] tests = [
            T("x"),
            T("y"),
            T("foobar")
        ];

        bool testLetStatement(Statement stmt, string name) {
            assert(stmt.tokenLiteral == "let", "stmt.tokenLiteral is not 'let'");
            auto letStmt = cast(LetStatement) stmt;
            assert(letStmt !is null, "stmt is not a LetStatement");
            assert(letStmt.name.value == name, format("letStmt.name.value not '%s'. got '%s'", name, letStmt.name.value));
            assert(letStmt.name.tokenLiteral == name, format("letStmt.name.tokenLiteral not '%s'. got '%s'", name, letStmt.name.tokenLiteral));
            return false;
        }

        foreach(i, t; tests) {
            auto stmt = program[i];
            testLetStatement(stmt, t.expectedIdentifier);
        }
    }

    {// RETURN
        string input = "return 5;" ~
        "return 10;" ~
        "return 993322;";

        auto l = Lexer(input);
        auto p = Parser(l);
        auto program = p.parseProgram();
        checkParserErrors(p);

        assert(program !is null, "parseProgram() returned null");
        assert(program.length == 3, "program.statements does not contain 3 statements");

        for (auto i = 0; i != program.length; ++i) {
            auto stmt = program[i];
            auto returnStmt = cast(ReturnStatement) stmt;
            assert(returnStmt !is null, format("stmt is not a ReturnStatement. got '%s'", returnStmt));
            assert(returnStmt.tokenLiteral == "return", format("returnStmt.tokenLiteral not 'return'. got '%s'", returnStmt.tokenLiteral));
        }
    }

    {// IDENTIFIERS
        string input = "foobar;";

        auto l = Lexer(input);
        auto p = Parser(l);
        auto program = p.parseProgram();
        checkParserErrors(p);

        assert(program !is null, "parseProgram() returned null");
        assert(program.length == 1, "program.statements does not contain 1 statements");

        auto expressionStmt = cast(ExpressionStatement) program[0];
        assert(expressionStmt !is null, format("expressionStmt is not an ExpressionStatement. got '%s'", expressionStmt));
        auto id = cast(Identifier) expressionStmt.expression;
        assert(id !is null, format("exp not Identifier. got '%s'", id));
        assert(id.value == "foobar");
        assert(id.tokenLiteral == "foobar");
    }

    {// INTEGER LITERALS
        string input = "5;";

        auto l = Lexer(input);
        auto p = Parser(l);
        auto program = p.parseProgram();
        checkParserErrors(p);

        assert(program !is null, "parseProgram() returned null");
        assert(program.length == 1, "program.statements does not contain 1 statements");

        auto expressionStmt = cast(ExpressionStatement) program[0];
        assert(expressionStmt !is null, format("expressionStmt is not an ExpressionStatement. got '%s'", expressionStmt));

        auto literal = cast(IntegerLiteral) expressionStmt.expression;
        assert(literal !is null, format("exp not IntegerLiteral. got '%s'", literal));
        assert(literal.value == 5);
        assert(literal.tokenLiteral == "5");
    }

    void testLiteral(T, N)(const(Expression) expression, T value) {
        import std.conv;
        auto n = cast(N) expression;
        assert(n, format("expected '%s', got '%s'", N.stringof, n));
        assert(n.value == value);
        assert(n.tokenLiteral == to!string(value));
    }

    void PrefixTestCore(T, N)(const(Statement) stmt, string operator, T value)
    {
        auto expressionStmt = cast(ExpressionStatement) stmt;
        assert(expressionStmt !is null, format("expressionStmt is not an ExpressionStatement!. got '%s'", expressionStmt));
        auto prefixExpression = cast(PrefixExpression) expressionStmt.expression;
        assert(prefixExpression !is null, format("expected PrefixExpression, got '%s'", prefixExpression));
        assert(prefixExpression.operator == operator);
        testLiteral!(T, N)(prefixExpression.right, value);
    }

    {// PREFIX EXPRESSIONS
        struct PrefixTest(T) { string input; string operator; T integerValue; }
        PrefixTest!long[] prefixTests = [
            { input: "!5;" , operator: "!", integerValue: 5 },
            { input: "-15;", operator: "-", integerValue: 15},
        ];

        foreach(tt; prefixTests) {
            auto l = Lexer(tt.input);
            auto p = Parser(l);
            auto program = p.parseProgram();
            checkParserErrors(p);

            assert(program !is null, "parseProgram() returned null");
            assert(program.length == 1, "program.statements does not contain 1 statements");
            PrefixTestCore!(long, IntegerLiteral)(program[0], tt.operator, tt.integerValue);
        }

        PrefixTest!bool[] prefixTests2 = [
            {"!true;" , "!", true },
            {"!false;", "!", false},
        ];

        foreach(tt; prefixTests2) {
            auto l = Lexer(tt.input);
            auto p = Parser(l);
            auto program = p.parseProgram();
            checkParserErrors(p);

            assert(program !is null, "parseProgram() returned null");
            assert(program.length == 1, "program.statements does not contain 1 statements");
            PrefixTestCore!(bool, BooleanLiteral)(program[0], tt.operator, tt.integerValue);
        }
    }

    void InfixTestCore0(T, N)(const(Expression) exp, T left, string operator, T right)
    {
        auto infixExpression = cast(InfixExpression) exp;
        assert(infixExpression !is null, format("expected InfixExpression, got '%s'", infixExpression));
        testLiteral!(T, N)(infixExpression.left, left);
        assert(infixExpression.operator == operator);
        testLiteral!(T, N)(infixExpression.right, right);

    }

    void InfixTestCore(T, N)(const(Statement) stmt, T left, string operator, T right)
    {
        auto expressionStmt = cast(ExpressionStatement) stmt;
        assert(expressionStmt !is null, format("expressionStmt is not an ExpressionStatement!. got '%s'", expressionStmt));

        InfixTestCore0!(T, N)(expressionStmt.expression, left, operator, right);
    }

    {// INFIX EXPRESSIONS
        struct InfixTest(T) { string input; T leftValue; string operator; T rightValue; }
        InfixTest!long[] infixTests = [
            { input: "5 + 5;", leftValue: 5, operator: "+" , rightValue: 5 },
            { input: "5 - 5;", leftValue: 5, operator: "-" , rightValue: 5 },
            { input: "5 * 5;", leftValue: 5, operator: "*" , rightValue: 5 },
            { input: "5 / 5;", leftValue: 5, operator: "/" , rightValue: 5 },
            { input: "5 > 5;", leftValue: 5, operator: ">" , rightValue: 5 },
            { input: "5 < 5;", leftValue: 5, operator: "<" , rightValue: 5 },
            { input: "5 == 5;", leftValue: 5, operator: "==", rightValue: 5},
            { input: "5 != 5;", leftValue: 5, operator: "!=", rightValue: 5},
        ];

        foreach(tt; infixTests) {
            auto l = Lexer(tt.input);
            auto p = Parser(l);
            auto program = p.parseProgram();
            checkParserErrors(p);

            assert(program !is null, "parseProgram() returned null");
            assert(program.length == 1, "program.statements does not contain 1 statements");

            InfixTestCore!(long, IntegerLiteral)(program[0], tt.leftValue, tt.operator, tt.rightValue);
        }

        InfixTest!bool[] infixTests2 = [
            { input: "true == true"  , true , "==", true  },
            { input: "true != false" , true , "!=", false },
            { input: "false == false", false, "==", false },
        ];

        foreach(tt; infixTests2) {
            auto l = Lexer(tt.input);
            auto p = Parser(l);
            auto program = p.parseProgram();
            checkParserErrors(p);

            assert(program !is null, "parseProgram() returned null");
            assert(program.length == 1, "program.statements does not contain 1 statements");

            InfixTestCore!(bool, BooleanLiteral)(program[0], tt.leftValue, tt.operator, tt.rightValue);
        }
    }

    { // OPERATOR PRECEDENCE
        struct OperatorPrecedenceTest { string input; string expected; }
        OperatorPrecedenceTest[] tests = [
            { input: "-a * b", expected: "((-a) * b)" },
            { input: "!-a", expected: "(!(-a))" },
            { input: "a + b + c", expected: "((a + b) + c)" },
            { input: "a + b - c", expected: "((a + b) - c)" },
            { input: "a * b * c", expected: "((a * b) * c)" },
            { input: "a * b / c", expected: "((a * b) / c)" },
            { input: "a + b / c", expected: "(a + (b / c))" },
            { input: "a + b * c + d / e - f", expected: "(((a + (b * c)) + (d / e)) - f)" },
            { input: "3 + 4; -5 * 5", expected: "(3 + 4)((-5) * 5)" },
            { input: "5 > 4 == 3 < 4", expected: "((5 > 4) == (3 < 4))" },
            { input: "5 < 4 != 3 > 4", expected: "((5 < 4) != (3 > 4))" },
            { input: "3 + 4 * 5 == 3 * 1 + 4 * 5", expected: "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))" },
            { input: "3 + 4 * 5 == 3 * 1 + 4 * 5", expected: "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))" },
            { input: "true", expected: "true" },
            { input: "false", expected: "false" },
            { input: "3 > 5 == false", expected: "((3 > 5) == false)" },
            { input: "3 < 5 == true", expected: "((3 < 5) == true)" },
            { input: "1 + (2 + 3) + 4", expected: "((1 + (2 + 3)) + 4)" },
            { input: "(5 + 5) * 2", expected: "((5 + 5) * 2)" },
            { input: "2 / (5 + 5)", expected: "(2 / (5 + 5))" },
            { input: "-(5 + 5)", expected: "(-(5 + 5))" },
            { input: "!(true == true)", expected: "(!(true == true))" },
        ];
        
        foreach(tt; tests) {
            import std.string : format;
            auto l = Lexer(tt.input);
            auto p = Parser(l);
            auto program = p.parseProgram();
            checkParserErrors(p);

            assert(program.toString == tt.expected, format("Got '%s', Expected '%s'", program.toString, tt.expected));
        }
    }

    { // IF EXPRESSION
        auto input = "if (x < y) { x }";
        auto l = Lexer(input);
        auto p = Parser(l);
        auto program = p.parseProgram();
        checkParserErrors(p);

        assert(program !is null);
        assert(program.length == 1);

        auto expressionStmt = cast(ExpressionStatement) program[0];
        assert(expressionStmt);

        auto ifExpression = cast(IfExpression) expressionStmt.expression;
        assert(ifExpression, format("Expected IfExpression, got '%s'", expressionStmt.expression));

        InfixTestCore0!(string, Identifier)(ifExpression.condition, "x", "<", "y");

        assert(ifExpression.consequence.length == 1);
        auto consequence = cast(ExpressionStatement) ifExpression.consequence[0];
        assert(consequence);

        testLiteral!(string, Identifier)(consequence.expression, "x");
        
        assert(ifExpression.alternative is null);

    }
}

