#include "lexer.hpp"
#include "parser.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cerr << "[ERROR] No input file." << std::endl;
        return 1;
    }
    
    Lexer lexer(argv[1]);
    Parser parser(lexer);

    parser.parseProgram();

    return 0;
}