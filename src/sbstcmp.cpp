#include "lexer.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cerr << "[ERROR] No input file." << std::endl;
        return 1;
    }

    std::string path(argv[1]);
    Lexer lexer;
    std::vector<Token> tokens = lexer.parse(path);

    // if (lastToken.type == TOKEN_TYPE::T_ERROR) {
    //     const std::string& error = std::get<std::string>(lastToken.value);
    //     throw std::runtime_error(error);
    // }
    
    for (const auto& token : tokens) {
        token.print();
    }
    
    return 0;
}