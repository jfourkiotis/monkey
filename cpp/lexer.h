#ifndef MONKEY_LEXER_H_INCLUDED
#define MONKEY_LEXER_H_INCLUDED

#include <string>
#include <cctype>
#include "token.h"

namespace lexer {
class Lexer final {
public:
    explicit Lexer(const std::string& inp)
        : input(inp), position(0), read_position(0), ch(0) { 
        readChar();
    }

    token::Token NextToken() {
        using token::Token;

        Token tok;

        skipWhitespace();

        switch (ch)
        {
        case '=':
            if (peekChar() == '=') {
                readChar();
                tok = Token{token::EQ, "=="};
            } else {
                tok = Token{token::ASSIGN, ch};
            }
            break;
        case '(':
            tok = Token{token::LPAREN, ch};
            break;
        case ')':
            tok = Token{token::RPAREN, ch};
            break;
        case '+':
            tok = Token{token::PLUS, ch};
            break;
        case '-':
            tok = Token{token::MINUS, ch};
            break;
        case '!':
            if (peekChar() == '=') {
                readChar();
                tok = Token{token::NOT_EQ, "!="};
            } else {
                tok = Token{token::BANG, ch};
            }
            break;
        case '/':
            tok = Token{token::SLASH, ch};
            break;
        case '*':
            tok = Token{token::ASTERISK, ch};
            break;
        case '<':
            tok = Token{token::LT, ch};
            break;
        case '>':
            tok = Token{token::GT, ch};
            break;
        case ';':
            tok = Token{token::SEMICOLON, ch};
            break;
        case ',':
            tok = Token{token::COMMA, ch};
            break;
        case '{':
            tok = Token{token::LBRACE, ch};
            break;
        case '}':
            tok = Token{token::RBRACE, ch};
            break;
        case 0:
            tok = Token{token::EOF_, ""};
            break;
        default:
            if (isLetter(ch)) {
                auto literal = readIdentifier();
                tok = Token{token::LookupIdent(literal), literal};
                return tok;
            } else if (std::isdigit(ch)) {
                tok = Token{token::INT, readNumber()};
                return tok;
            } else {
                tok = Token{token::ILLEGAL, ch};
            }
        }

        readChar();

        return tok;
    }
private:
    void readChar() {
        if (read_position >= input.size()) {
            ch = 0; // we haven't read anything OR we are at end of file
        } else {
            ch = input[read_position];
        }
        position = read_position;
        ++read_position;
    }

    char peekChar() const {
        if (read_position >= input.size()) {
            return 0;
        } else {
            return input[read_position];
        }
    }

    std::string readIdentifier() {
        auto pos = position;
        while (isLetter(ch)) {
            readChar();
        }
        return input.substr(pos, position - pos);
    }

    std::string readNumber() {
        auto pos = position;
        while (std::isdigit(ch)) {
            readChar();
        }
        return input.substr(pos, position - pos);
    }

    void skipWhitespace() {
        while (std::isspace(ch)) {
            readChar();
        }
    }

    bool isLetter(char c) const {
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
    }

    std::string input;
    int position; // current position in input (points to current char)
    int read_position; // current reading position in input (after current char)
    char ch; // current char under examination
};
}//~ lexer

#endif // MONKEY_LEXER_H_INCLUDED

