module token;

alias TokenType = string;

struct Token {
    TokenType type;
    string literal;
}

immutable(TokenType) ILLEGAL = "ILLEGAL";
immutable(TokenType) EOF     = "EOF";

// Identifiers + literals
immutable(TokenType) IDENT   = "IDENT"; // add, foobar, x, y, ...
immutable(TokenType) INT     = "INT";   // 1343456

// Operators
immutable(TokenType) ASSIGN  = "=";
immutable(TokenType) PLUS    = "+";
immutable(TokenType) MINUS   = "-";
immutable(TokenType) BANG    = "!";
immutable(TokenType) ASTERISK = "*";
immutable(TokenType) SLASH   = "/";

immutable(TokenType) LT = "<";
immutable(TokenType) GT = ">";
immutable(TokenType) EQ = "==";
immutable(TokenType) NOT_EQ = "!=";

// Delimiters
immutable(TokenType) COMMA   = ",";
immutable(TokenType) SEMICOLON = ";";

immutable(TokenType) LPAREN  = "(";
immutable(TokenType) RPAREN  = ")";
immutable(TokenType) LBRACE  = "{";
immutable(TokenType) RBRACE  = "}";

// Keywords
immutable(TokenType) FUNCTION = "FUNCTION";
immutable(TokenType) LET    = "LET";
immutable(TokenType) TRUE   = "TRUE";
immutable(TokenType) FALSE  = "FALSE";
immutable(TokenType) IF     = "IF";
immutable(TokenType) ELSE   = "ELSE";
immutable(TokenType) RETURN = "RETURN";

immutable TokenType[string] keywords;

static this() {
    keywords["fn"]   = FUNCTION;
    keywords["let"]  = LET;
    keywords["true"] = TRUE;
    keywords["false"]= FALSE;
    keywords["if"]   = IF;
    keywords["else"] = ELSE;
    keywords["return"] = RETURN; 
}

TokenType lookupIdent(string ident) {
    auto p = ident in keywords;
    if (p !is null) {
        return *p;
    }
    return IDENT;
}
