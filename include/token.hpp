#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <variant>
#include <string>
#include <array>

using TOKEN_VALUE = std::variant<std::monostate, char, std::string>;

enum class TOKEN_TYPE {
    MAIN = 1,
    INT = 2,
    SHORT = 3,
    LONG = 4,
    CHAR = 5,
    TYPEDEF = 6,
    FOR = 7,
    IDENT = 20,
    CONST_DEC = 30,
    CONST_HEX = 31,
    CONST_SYMB = 32,
    CONST_STR = 33,
    COMMA = 40,
    SEMICOLON = 41,
    LPAREN = 42,
    RPAREN = 43,
    LBRACE = 44,
    RBRACE = 45,
    LBRACKET = 46,
    RBRACKET = 47,
    LT = 50,
    LE = 51,
    GT = 52,
    GE = 53,
    EQ = 54,
    NEQ = 55,
    BLS = 56,
    BRS = 57,
    PLUS = 58,
    MINUS = 59,
    MULT = 60,
    DIV = 61,
    MOD = 62,
    ASSIGN = 63,
    END = 100,
    ERROR = 200,
};

// Structure that represents a single token
struct Token {
public:
    Token();
    Token(TOKEN_TYPE type, size_t lineStart, size_t lineEnd, size_t columnStart, size_t columnEnd);
    Token(TOKEN_TYPE type, char value, size_t lineStart, size_t lineEnd, size_t columnStart, size_t columnEnd);
    Token(TOKEN_TYPE type, std::string&& value, size_t lineStart, size_t lineEnd, size_t columnStart, size_t columnEnd);

    void print() const;

public:
    TOKEN_TYPE type;   // Type of a token
    TOKEN_VALUE value; // Value of a token that can take different types

    size_t m_lineStart, m_lineEnd;
    size_t m_columnStart, m_columnEnd;
};

#endif // TOKEN_HPP