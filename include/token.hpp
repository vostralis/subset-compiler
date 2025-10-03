#include <variant>
#include <string>

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
    PAR_OPEN = 42,
    PAR_CLOSE = 43,
    BRACE_OPEN = 44,
    BRACE_CLOSE = 45,
    BRACKET_OPEN = 46,
    BRACKET_CLOSE = 47,
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
    Token(TOKEN_TYPE type);
    Token(TOKEN_TYPE type, char value);
    Token(TOKEN_TYPE type, std::string&& value);

    void print() const;

public:
    TOKEN_TYPE type;   // Type of a token
    TOKEN_VALUE value; // Value of a token that can take different types
};