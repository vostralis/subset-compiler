#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"

#include <array>

class Parser {
public:
    Parser(Lexer& lexer);
    ~Parser();

    void parseProgram();  // P -> P D_s | e

    enum class PARSER_ERROR {
        UNEXPECTED_TOKEN,
        UNEXPECTED_EOF,
        INVALID_EXPRESSION,
        MISSING_TYPE_SPECIFIER,
        MISSING_LPAREN,
        MISSING_RPAREN,
        MISSING_LBRACE,
        MISSING_RBRACE,
        MISSING_IDENTIFIER,
        MISSING_SEMICOLON,
        MISSING_ASSIGN,
        MISSING_LBRACKET,
        MISSING_RBRACKET,
        MISSING_EXPRESSION
    };

private:
    void parseDescription();
    bool isDescriptionStart(TOKEN_TYPE type) const;
    void parseBlock();
    bool isStatementOrDeclarationStart(TOKEN_TYPE type) const;
    bool isDeclaration(TOKEN_TYPE type) const; // Can be removed later
    void parseTypedef();
    void parseDeclaration();
    void parseType();
    void parseVariableList();
    void parseStatement();
    void parseForStatement();
    void parseAssignmentStatement();
    void parseEqualityExpression();
    void parseComparisonExpression();
    void parseBitwiseShiftExpression();
    void parseAdditiveExpression();
    void parseMultiplicativeExpression();
    void parseUnaryExpression();
    bool isConstant(TOKEN_TYPE type) const;

    Token lookahead(size_t distance = 0) const;
    void consume();
    void match(TOKEN_TYPE expected, PARSER_ERROR mismatchCode = PARSER_ERROR::UNEXPECTED_TOKEN);
    void error(PARSER_ERROR code, const Token& found);
    void tokenDebug() const;

private:
    Lexer& lexer;

    static constexpr size_t BUFFER_SIZE = 8;
    std::array<Token, BUFFER_SIZE> m_lookaheadBuffer;
    size_t m_bufferPos;
    size_t m_previousLineEnd, m_previousColumnEnd;
};

#endif // PARSER_HPP