#include "token.hpp"

#include <iostream>

Token::Token() = default;

Token::Token(TOKEN_TYPE type, size_t lineStart, size_t lineEnd, size_t columnStart, size_t columnEnd) :
    type(type), value(std::monostate()), 
    m_lineStart(lineStart), m_lineEnd(lineEnd),
    m_columnStart(columnStart), m_columnEnd(columnEnd)
{}

Token::Token(TOKEN_TYPE type, char value, size_t lineStart, size_t lineEnd, size_t columnStart, size_t columnEnd) :
    type(type), value(value), 
    m_lineStart(lineStart), m_lineEnd(lineEnd),
    m_columnStart(columnStart), m_columnEnd(columnEnd)
{}

Token::Token(TOKEN_TYPE type, std::string&& value, size_t lineStart, size_t lineEnd, size_t columnStart, size_t columnEnd) :
    type(type), value(value), 
    m_lineStart(lineStart), m_lineEnd(lineEnd),
    m_columnStart(columnStart), m_columnEnd(columnEnd)
{}

void Token::print() const {
    switch (type) {
        case TOKEN_TYPE::MAIN:
        {
            std::cout << "T_MAIN" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::INT:
        {
            std::cout << "T_INT" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::SHORT:
        {
            std::cout << "T_SHORT" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::LONG:
        {
            std::cout << "T_LONG" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::CHAR:
        {
            std::cout << "T_CHAR" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::TYPEDEF:
        {
            std::cout << "T_TYPEDEF" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::FOR:
        {
            std::cout << "T_FOR" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
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
            std::cout << "T_COMMA" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::SEMICOLON:
        {
            std::cout << "T_SEMICOLON" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::PAR_OPEN:
        {
            std::cout << "T_PAR_OPEN" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::PAR_CLOSE:
        {
            std::cout << "T_PAR_CLOSE" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::BRACE_OPEN:
        {
            std::cout << "T_BRACE_OPEN" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::BRACE_CLOSE:
        {
            std::cout << "T_BRACE_CLOSE" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::BRACKET_OPEN:
        {
            std::cout << "T_BRACKET_OPEN" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::BRACKET_CLOSE:
        {
            std::cout << "T_BRACKET_CLOSE" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::LT:
        {
            std::cout << "T_LT" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::LE:
        {
            std::cout << "T_LE" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::GT:
        {
            std::cout << "T_GT" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::GE:
        {
            std::cout << "T_GE" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::EQ:
        {
            std::cout << "T_EQ" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::NEQ:
        {
            std::cout << "T_NEQ" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::BLS:
        {
            std::cout << "T_BLS" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::BRS:
        {
            std::cout << "T_BRS" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::PLUS:
        {
            std::cout << "T_PLUS" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::MINUS:
        {
            std::cout << "T_MINUS" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::MULT:
        {
            std::cout << "T_MULT" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::DIV:
        {
            std::cout << "T_DIV" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::MOD:
        {
            std::cout << "T_MOD" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::ASSIGN:
        {
            std::cout << "T_ASSIGN" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
            break;
        }
        case TOKEN_TYPE::END:
        {
            std::cout << "T_END" << ' ' << m_columnStart << ' ' << m_columnEnd << '\n';
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