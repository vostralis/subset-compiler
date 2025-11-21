#include "lexer.hpp"
#include "parser.hpp"
#include "analyzer.hpp"
#include "ast_printer.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cerr << "[ERROR] No input file." << std::endl;
        return 1;
    }

    bool displayTree = false;
    std::string input;

    if (std::string(argv[1]) == "-T") {
        displayTree = true;
        input = std::string(argv[2]);
    } else {
        input = std::string(argv[1]);
    }
    
    Lexer lexer(input);
    
    Parser parser(lexer);
    auto root = parser.parseProgram();

    if (root) {
        Analyzer analyzer(input);
        analyzer.analyze(*root);

        if (displayTree) {
            ASTPrinter printer;
            printer.print(*root);
        }
    }

    return 0;
}