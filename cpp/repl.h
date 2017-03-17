#ifndef MONKEY_REPL_H_INCLUDED
#define MONKEY_REPL_H_INCLUDED

#include <string>
#include <iostream>

#include "token.h"
#include "lexer.h"
#include "parser.h"

const std::string PROMPT = ">> ";
const std::string MONKEY_FACE = R"(
            __,__
   .--.  .-"     "-.  .--.
  / .. \/  .-. .-.  \/ .. \
 | |  '|  /   Y   \  |'  | |
 | \   \  \ 0 | 0 /  /   / |
  \ '- ,\.-"""""""-./, -' /
   ''-' /_   ^ ^   _\ '-''
       |  \._   _./  |
       \   \ '~' /   /
        '._ '-=-' _.'
           '-----'
)";

void PrintParserErrors(std::ostream& out, const Parser::ErrorList &errors)
{
    out << MONKEY_FACE;
    out << "Woops! We ran into some monkey business here!" << '\n';
    out << " parser errors:\n";
    for (const auto& err : errors) {
        out << '\t' << err << '\n';
    }
}

inline void Start(std::istream& in, std::ostream& out) {
    std::string line;
    while (true) {
        out << PROMPT;
        if (!std::getline(in, line)) return;

        auto l = lexer::Lexer{line};
        auto p = Parser{l};
        auto program = p.ParseProgram();

        Parser::ErrorList errors = p.Errors();
        if (!errors.empty()) {
            PrintParserErrors(out, errors);
            continue;
        }

        out << program->ToString() << '\n';
    }
}

#endif // MONKEY_REPL_H_INCLUDED

