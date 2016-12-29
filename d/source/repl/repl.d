module repl;

import std.stdio;
import lexer : Lexer;
import token;

immutable PROMPT = ">> ";

void start(File input, File output) {
    while (true) {
        write(PROMPT);
        string line = input.readln();
        auto l = Lexer(line);
        for (auto tok = l.nextToken(); tok.type != token.EOF; tok = l.nextToken()) {
            output.writeln(tok);
        }
    }
}