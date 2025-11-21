#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"

#include <array>

class Parser {
public:
    Parser(Lexer& lexer);
    ~Parser();

    std::unique_ptr<ProgramNode> parseProgram();  // P -> P D_s | e

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
    bool isDescriptionStart(TOKEN_TYPE type) const;
    std::unique_ptr<DeclarationNode> parseMainFunction();
    std::unique_ptr<CompoundStatementNode> parseCompoundStatement();
    bool isStatementOrDeclarationStart(TOKEN_TYPE type) const;
    bool isDeclaration(TOKEN_TYPE type) const; // Can be removed later
    std::unique_ptr<TypedefNode> parseTypedef();
    std::vector<std::unique_ptr<DeclarationNode>> parseDeclaration();
    ParsedType parseTypeSpecifier();

    std::vector<std::unique_ptr<DeclarationNode>> parseVariableList(const ParsedType& typeInfo);
    std::unique_ptr<DeclarationNode> parseSingleVariableDeclaration(const ParsedType& typeInfo);
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<ForNode> parseForStatement();
    std::unique_ptr<AssignmentNode> parseAssignmentStatement();
    std::unique_ptr<ExpressionNode> parseEqualityExpression();
    std::unique_ptr<ExpressionNode> parseComparisonExpression();
    std::unique_ptr<ExpressionNode> parseBitwiseShiftExpression();
    std::unique_ptr<ExpressionNode> parseAdditiveExpression();
    std::unique_ptr<ExpressionNode> parseMultiplicativeExpression();
    std::unique_ptr<ExpressionNode> parseUnaryExpression();
    bool isConstant(TOKEN_TYPE type) const;

    Token lookahead(size_t distance = 0) const;
    Token consume();
    void match(TOKEN_TYPE expected, PARSER_ERROR mismatchCode = PARSER_ERROR::UNEXPECTED_TOKEN);
    void error(PARSER_ERROR code, const Token& found);
    void tokenDebug() const;

private:
    Lexer& lexer;

    static constexpr size_t BUFFER_SIZE = 8;
    std::array<Token, BUFFER_SIZE> m_lookaheadBuffer;
    Token consumedToken;
    size_t m_bufferPos;
    size_t m_previousLineEnd, m_previousColumnEnd;
};

#endif // PARSER_HPP