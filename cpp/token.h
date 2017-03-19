#ifndef MONKEY_TOKEN_HPP_INCLUDED
#define MONKEY_TOKEN_HPP_INCLUDED

#include <string>
#include <map>

namespace token {
using TokenType = std::string;

const TokenType ILLEGAL   = "ILLEGAL";
const TokenType EOF_      = "EOF";
// Identifiers
const TokenType IDENT     = "IDENT";
const TokenType INT       = "INT";
// Operators
const TokenType ASSIGN    = "=";
const TokenType PLUS      = "+";
const TokenType MINUS     = "-";
const TokenType BANG      = "!";
const TokenType ASTERISK  = "*";
const TokenType SLASH     = "/";
const TokenType LT        = "<";
const TokenType GT        = ">";

const TokenType EQ        = "==";
const TokenType NOT_EQ    = "!=";
// Delimiters
const TokenType COMMA     = ",";
const TokenType SEMICOLON = ";";
const TokenType LPAREN    = "(";
const TokenType RPAREN    = ")";
const TokenType LBRACE    = "{";
const TokenType RBRACE    = "}";
// Keywords
const TokenType FUNCTION  = "FUNCTION";
const TokenType LET       = "LET";
const TokenType TRUE_     = "TRUE";
const TokenType FALSE_    = "FALSE";
const TokenType IF        = "IF";
const TokenType ELSE      = "ELSE";
const TokenType RETURN    = "RETURN";

struct Token final {
    Token() : type(ILLEGAL), literal("") {}
    Token(TokenType t, char c) : type(t), literal(1, c) {}
    Token(TokenType t, const std::string& s) : type(t), literal(s) {}
    TokenType type;
    std::string literal;
};

const std::map<std::string, TokenType> KEYWORDS = {
    {"fn"   , FUNCTION},
    {"let"  , LET},
    {"true" , TRUE_},
    {"false", FALSE_},
    {"if"   , IF},
    {"else" , ELSE},
    {"return", RETURN},
};

inline TokenType LookupIdent(const std::string& ident) {
    std::map<std::string, TokenType>::const_iterator i = 
        KEYWORDS.find(ident);
    return (i != KEYWORDS.end()) ? i->second : IDENT;
}
}//~ token

#endif

