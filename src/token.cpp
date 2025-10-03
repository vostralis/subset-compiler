#include "token.hpp"

#include <iostream>

Token::Token(TOKEN_TYPE type) :
    type(type), value(std::monostate())
{}

Token::Token(TOKEN_TYPE type, char value) :
    type(type), value(value)
{}

Token::Token(TOKEN_TYPE type, std::string&& value) :
    type(type), value(value)
{}

void Token::print() const {
    switch (type) {
        case TOKEN_TYPE::MAIN:
        {
            std::cout << "T_MAIN\n";
            break;
        }
        case TOKEN_TYPE::INT:
        {
            std::cout << "T_INT\n";
            break;
        }
        case TOKEN_TYPE::SHORT:
        {
            std::cout << "T_SHORT\n";
            break;
        }
        case TOKEN_TYPE::LONG:
        {
            std::cout << "T_LONG\n";
            break;
        }
        case TOKEN_TYPE::CHAR:
        {
            std::cout << "T_CHAR\n";
            break;
        }
        case TOKEN_TYPE::TYPEDEF:
        {
            std::cout << "T_TYPEDEF\n";
            break;
        }
        case TOKEN_TYPE::FOR:
        {
            std::cout << "T_FOR\n";
            break;
        }
        case TOKEN_TYPE::IDENT:
        {
            std::cout << "T_IDENT: " << std::get<std::string>(value) << '\n';
            break;
        }
        case TOKEN_TYPE::CONST_DEC:
        {
            std::cout << "T_CONST_DEC: " << std::get<std::string>(value) << '\n';
            break;
        }
        case TOKEN_TYPE::CONST_HEX: {
            std::cout << "T_CONST_HEX: " << std::hex << std::get<std::string>(value) << '\n';
            break;
        }
        case TOKEN_TYPE::CONST_SYMB:
        {
            std::cout << "T_CONST_SYMB: " << std::get<char>(value) << '\n';
            break;
        }
        case TOKEN_TYPE::CONST_STR:
        {
            std::cout << "T_CONST_STR: " << std::get<std::string>(value) << '\n';
            break;
        }
        case TOKEN_TYPE::COMMA:
        {
            std::cout << "T_COMMA\n";
            break;
        }
        case TOKEN_TYPE::SEMICOLON:
        {
            std::cout << "T_SEMICOLON\n";
            break;
        }
        case TOKEN_TYPE::PAR_OPEN:
        {
            std::cout << "T_PAR_OPEN\n";
            break;
        }
        case TOKEN_TYPE::PAR_CLOSE:
        {
            std::cout << "T_PAR_CLOSE\n";
            break;
        }
        case TOKEN_TYPE::BRACE_OPEN:
        {
            std::cout << "T_BRACE_OPEN\n";
            break;
        }
        case TOKEN_TYPE::BRACE_CLOSE:
        {
            std::cout << "T_BRACE_CLOSE\n";
            break;
        }
        case TOKEN_TYPE::BRACKET_OPEN:
        {
            std::cout << "T_BRACKET_OPEN\n";
            break;
        }
        case TOKEN_TYPE::BRACKET_CLOSE:
        {
            std::cout << "T_BRACKET_CLOSE\n";
            break;
        }
        case TOKEN_TYPE::LT:
        {
            std::cout << "T_LT\n";
            break;
        }
        case TOKEN_TYPE::LE:
        {
            std::cout << "T_LE\n";
            break;
        }
        case TOKEN_TYPE::GT:
        {
            std::cout << "T_GT\n";
            break;
        }
        case TOKEN_TYPE::GE:
        {
            std::cout << "T_GE\n";
            break;
        }
        case TOKEN_TYPE::EQ:
        {
            std::cout << "T_EQ\n";
            break;
        }
        case TOKEN_TYPE::NEQ:
        {
            std::cout << "T_NEQ\n";
            break;
        }
        case TOKEN_TYPE::BLS:
        {
            std::cout << "T_BLS\n";
            break;
        }
        case TOKEN_TYPE::BRS:
        {
            std::cout << "T_BRS\n";
            break;
        }
        case TOKEN_TYPE::PLUS:
        {
            std::cout << "T_PLUS\n";
            break;
        }
        case TOKEN_TYPE::MINUS:
        {
            std::cout << "T_MINUS\n";
            break;
        }
        case TOKEN_TYPE::MULT:
        {
            std::cout << "T_MULT\n";
            break;
        }
        case TOKEN_TYPE::DIV:
        {
            std::cout << "T_DIV\n";
            break;
        }
        case TOKEN_TYPE::MOD:
        {
            std::cout << "T_MOD\n";
            break;
        }
        case TOKEN_TYPE::ASSIGN:
        {
            std::cout << "T_ASSIGN\n";
            break;
        }
        case TOKEN_TYPE::END:
        {
            std::cout << "T_END\n";
            break;
        }
        case TOKEN_TYPE::ERROR:
        {
            std::cout << "T_ERROR: " << std::get<std::string>(value) << '\n';
            break;
        }

        default:
            throw std::runtime_error("[ERROR]: Invalid token type.");
    } 
}