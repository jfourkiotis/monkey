#include <iostream>
#include "repl.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello! This is the Monkey programming language!\n";
    std::cout << "Feel free to type in commands\n";
    Start(std::cin, std::cout);
    return 0;
}

