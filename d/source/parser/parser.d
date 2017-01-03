module parser;

import token;
import lexer;
import ast;

struct Parser {
private:
    Lexer lexer_;
    Token curToken_;
    Token peekToken_;
    string[] errors_;

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
        default:
            return null;
        }
    }

    Statement parseLetStatement() {
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
public:
    @disable this();

    this(Lexer lexer) {
        lexer_ = lexer;
        nextToken();
        nextToken();
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

    string input = "let x = 5;" ~
    "let y = 10;" ~
    "let foobar = 838383;";

    auto l = Lexer(input);
    auto p = Parser(l);
    auto program = p.parseProgram();


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

