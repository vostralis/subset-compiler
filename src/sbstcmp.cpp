#include "lexer.hpp"
#include "parser.hpp"
#include "analyzer.hpp"
#include "ast_printer.hpp"
#include "interpreter.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cerr << "[ERROR] No input file." << std::endl;
        return 1;
    }

    bool displayTree = false;
    bool isInterpretationEnabled = false;
    std::string filepath;

    for (int i = 1; i < argc; ++i) {
        auto arg = std::string(argv[i]);

        if (arg == "-T") displayTree = true;
        else if (arg == "--int") isInterpretationEnabled = true;
        else filepath = arg; 
    }

    Lexer lexer(filepath);
    
    Parser parser(lexer);
    auto root = parser.parseProgram();

    if (!root) {
        std::cerr << "[ERROR]: Abstract syntax tree parsing failed." << std::endl;
        return 1;
    }

    Analyzer analyzer(filepath);
    SymbolTable& table = analyzer.analyze(*root);

    if (displayTree) {
        ASTPrinter printer;
        printer.print(*root);
    }

    if (isInterpretationEnabled) {
        Interpreter interpreter(filepath, table);
        interpreter.interprete(*root);
    }

    return 0;
}