module repl;

import std.stdio;
import lexer : Lexer;
import parser : Parser;
import token;

immutable PROMPT = ">> ";
immutable MONKEY_FACE = `            __,__
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
`; 

void start(File input, File output) {
    while (true) {
        output.write(PROMPT);
        string line = input.readln();
        if (line is null) { 
            return;
        }
        auto l = Lexer(line);
        auto p = Parser(l);
        auto program = p.parseProgram();
        if (p.errors.length != 0) {
            printParserErrors(output, p.errors);
            continue;
        }
        output.write(program.toString);
        output.write('\n');
    }
}

void printParserErrors(File output, string[] errors) {
    output.writeln(MONKEY_FACE); 
    output.writeln("Woops! We ran into some monkey business here!");
    output.writeln(" parser errors:");
    foreach (err; errors) {
        output.writefln("\t%s", err);
    }
}
