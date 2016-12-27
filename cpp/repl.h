#ifndef MONKEY_REPL_H_INCLUDED
#define MONKEY_REPL_H_INCLUDED

#include <string>
#include <iostream>

#include "token.h"
#include "lexer.h"

const std::string PROMPT = ">> ";

inline void Start(std::istream& in, std::ostream& out) {
    std::string line;
    while (true) {
        out << PROMPT;
        if (!std::getline(in, line)) return;

        auto l = lexer::Lexer{line};
        for (auto tok = l.NextToken(); tok.type != token::EOF_; tok = l.NextToken()) {
            out << "{Type:" << tok.type << " Literal:" << tok.literal << "}\n";
        }
    }
}

#endif // MONKEY_REPL_H_INCLUDED

