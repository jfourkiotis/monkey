import std.stdio;
import repl;

void main()
{
    writeln("Hello! This is the Monkey programming language!");
    writeln("Feel free to type in commands");
    repl.start(stdin, stdout);
}
