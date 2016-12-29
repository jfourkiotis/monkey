module lexer;

import std.conv;
import std.ascii : isDigit;
import token;

struct Lexer {
    string input;
    int position;
    int readPosition;
    char ch;

    void readChar() {
        if (readPosition >= input.length) {
            ch = 0;
        } else {
            ch = input[readPosition];
            position = readPosition;
            ++readPosition;
        }
    }

    char peekChar() {
        if (readPosition >= input.length) {
            return 0;
        } else {
            return input[readPosition];
        }
    }

    bool isLetter(char ch) {
        return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
    }

    string readIdentifier() {
        auto pos = position;
        while (isLetter(ch)) {
            readChar();
        }
        return input[pos..position];
    }

    void skipWhitespace() {
        while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            readChar();
        }
    }

    string readNumber() {
        auto pos = position;
        while (isDigit(ch)) {
            readChar();
        }
        return input[pos..position];
    }

    @disable this();

    // public
    this(string inp) {
        input = inp;
        readChar();
    }

    token.Token nextToken() {
        token.Token tok;

        skipWhitespace();

        switch (ch) {
            case '=': 
                if (peekChar() == '=') {
                    readChar();
                    tok = token.Token(EQ, "==");
                } else {
                    tok = token.Token(ASSIGN, to!string(ch)); 
                }
                break;
            case '(': 
                tok = token.Token(LPAREN, to!string(ch)); 
                break;
            case ')': 
                tok = token.Token(RPAREN, to!string(ch)); 
                break;
            case '+': 
                tok = token.Token(PLUS, to!string(ch)); 
                break;
            case '-': 
                tok = token.Token(MINUS, to!string(ch)); 
                break;
            case '!': 
                if (peekChar() == '=') {
                    readChar();
                    tok = token.Token(NOT_EQ, "!=");
                } else {
                    tok = token.Token(BANG, to!string(ch)); 
                }
                break;
            case '/': 
                tok = token.Token(SLASH, to!string(ch)); 
                break;
            case '*': 
                tok = token.Token(ASTERISK, to!string(ch)); 
                break;
            case '<': 
                tok = token.Token(LT, to!string(ch)); 
                break;
            case '>': 
                tok = token.Token(GT, to!string(ch)); 
                break;
            case ';': 
                tok = token.Token(SEMICOLON, to!string(ch)); 
                break;
            case ',': 
                tok = token.Token(COMMA, to!string(ch)); 
                break;
            case '{': 
                tok = token.Token(LBRACE, to!string(ch)); 
                break;
            case '}': 
                tok = token.Token(RBRACE, to!string(ch)); 
                break;
            case 0  : 
                tok.literal = "";
                tok.type = EOF;
                break;
            default:
                if (isLetter(ch)) {
                    auto literal = readIdentifier();
                    tok = token.Token(token.lookupIdent(literal), literal);
                    return tok;
                } else if (isDigit(ch)) {
                    return token.Token(INT, readNumber());
                } else {
                    tok = token.Token(ILLEGAL, to!string(ch));
                }
        }

        readChar();

        return tok;
    }

}

unittest {
    import std.stdio : writeln;

    string input = "let five = 5;" ~
    "let ten = 10;" ~
    "   "~
    "let add = fn(x, y) {" ~
    "  x + y;" ~
    "};" ~
    "  " ~
    "let result = add(five, ten);" ~
    "!-/*5;" ~
    "5 < 10 > 5;" ~
    "   " ~
    "if (5 < 10) {" ~
    "  return true; " ~
    "} else {" ~
    "  return false;" ~
    "}" ~
    " " ~
    "10 == 10;" ~
    "10 != 9;";

    struct T { token.TokenType expectedType; string expectedLiteral; }
    immutable(T[]) tests = [
        T(LET, "let"),
        T(IDENT, "five"),
        T(ASSIGN, "="),
        T(INT, "5"),
        T(SEMICOLON, ";"),
        T(LET, "let"),
        T(IDENT, "ten"),
        T(ASSIGN, "="),
        T(INT, "10"),
        T(SEMICOLON, ";"),
        T(LET, "let"),
        T(IDENT, "add"),
        T(ASSIGN, "="),
        T(FUNCTION, "fn"),
        T(LPAREN, "("),
        T(IDENT, "x"),
        T(COMMA, ","),
        T(IDENT, "y"),
        T(RPAREN, ")"),
        T(LBRACE, "{"),
        T(IDENT, "x"),
        T(PLUS, "+"),
        T(IDENT, "y"),
        T(SEMICOLON, ";"),
        T(RBRACE, "}"),
        T(SEMICOLON, ";"),
        T(LET, "let"),
        T(IDENT, "result"),
        T(ASSIGN, "="),
        T(IDENT, "add"),
        T(LPAREN, "("),
        T(IDENT, "five"),
        T(COMMA, ","),
        T(IDENT, "ten"),
        T(RPAREN, ")"),
        T(SEMICOLON, ";"),
        T(BANG, "!"),
        T(MINUS, "-"),
        T(SLASH, "/"),
        T(ASTERISK, "*"),
        T(INT, "5"),
        T(SEMICOLON, ";"),
        T(INT, "5"),
        T(LT, "<"),
        T(INT, "10"),
        T(GT, ">"),
        T(INT, "5"),
        T(SEMICOLON, ";"),
        T(IF, "if"),
        T(LPAREN, "("),
        T(INT, "5"),
        T(LT, "<"),
        T(INT, "10"),
        T(RPAREN, ")"),
        T(LBRACE, "{"),
        T(RETURN, "return"),
        T(TRUE, "true"),
        T(SEMICOLON, ";"),
        T(RBRACE, "}"),
        T(ELSE, "else"),
        T(LBRACE, "{"),
        T(RETURN, "return"),
        T(FALSE, "false"),
        T(SEMICOLON, ";"),
        T(RBRACE, "}"),
        T(INT, "10"),
        T(EQ, "=="),
        T(INT, "10"),
        T(SEMICOLON, ";"),
        T(INT, "10"),
        T(NOT_EQ, "!="),
        T(INT, "9"),
        T(SEMICOLON, ";"),
        T(EOF, ""),
    ];

    auto l = Lexer(input);
    foreach (test; tests) {
        immutable tok = l.nextToken();
        //writeln(tok.literal, test.expectedLiteral, tok.type, test.expectedType);
        assert(test.expectedType == tok.type);
        assert(test.expectedLiteral == tok.literal);
    }
}
