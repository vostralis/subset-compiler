#include "parser.hpp"

#include <iostream>
#include <format>

Parser::Parser(Lexer& lexer) :
    lexer(lexer), m_bufferPos(0)
{
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        m_lookaheadBuffer[i] = lexer.getNextToken();
        
        if (m_lookaheadBuffer[i].type == TOKEN_TYPE::END) {
            break;
        }
    }    
}

Parser::~Parser() = default;

Token Parser::lookahead(size_t distance) const {
    return m_lookaheadBuffer[(m_bufferPos + distance) % BUFFER_SIZE];
}

void Parser::consume() {
    Token token = lookahead();
    m_previousLineEnd = token.m_lineEnd;
    m_previousColumnEnd = token.m_columnEnd;

    m_lookaheadBuffer[m_bufferPos] = lexer.getNextToken();
    m_bufferPos = (m_bufferPos + 1) % BUFFER_SIZE;
}

void Parser::match(TOKEN_TYPE expected, PARSER_ERROR mismatchCode) {
    Token found = lookahead();

    if (found.type == expected) {
        //lookahead().print();
        consume();
    } else {
        error(mismatchCode, found);
    }
}

void Parser::parseProgram() {
    while (isDescriptionStart(lookahead().type)) {
        parseDescription();
    }

    match(TOKEN_TYPE::END);
}

bool Parser::isDescriptionStart(TOKEN_TYPE type) const {
    return type == TOKEN_TYPE::TYPEDEF || type == TOKEN_TYPE::INT ||
           type == TOKEN_TYPE::SHORT || type == TOKEN_TYPE::LONG ||
           type == TOKEN_TYPE::CHAR || type == TOKEN_TYPE::IDENT;
}

void Parser::parseDescription() {
    // **
    if (lookahead().type == TOKEN_TYPE::INT && lookahead(1).type == TOKEN_TYPE::MAIN) {
        match(TOKEN_TYPE::INT, PARSER_ERROR::MISSING_TYPE_SPECIFIER);
        match(TOKEN_TYPE::MAIN, PARSER_ERROR::UNEXPECTED_TOKEN);
        match(TOKEN_TYPE::PAR_OPEN, PARSER_ERROR::MISSING_LPAREN);
        match(TOKEN_TYPE::PAR_CLOSE, PARSER_ERROR::MISSING_RPAREN);
        match(TOKEN_TYPE::BRACE_OPEN, PARSER_ERROR::MISSING_LBRACE);
        parseBlock();
        match(TOKEN_TYPE::BRACE_CLOSE, PARSER_ERROR::MISSING_RBRACE);
    } else if (lookahead().type == TOKEN_TYPE::TYPEDEF) {
        parseTypedef();
    } else {
        parseDeclaration();
    }
}

bool Parser::isStatementOrDeclarationStart(TOKEN_TYPE type) const {
    switch (type) {
        case TOKEN_TYPE::INT:
        case TOKEN_TYPE::SHORT:
        case TOKEN_TYPE::LONG:
        case TOKEN_TYPE::CHAR:
        case TOKEN_TYPE::IDENT:
        case TOKEN_TYPE::FOR:
        case TOKEN_TYPE::SEMICOLON:
        case TOKEN_TYPE::BRACE_OPEN:
            return true;
        default:
            return false;
    }
}

bool Parser::isDeclaration(TOKEN_TYPE type) const {
    switch (type) {
        case TOKEN_TYPE::INT:
        case TOKEN_TYPE::SHORT:
        case TOKEN_TYPE::LONG:
        case TOKEN_TYPE::CHAR:
            return true;
        default:
            return false;
    }
}

void Parser::parseBlock() {
    while (isStatementOrDeclarationStart(lookahead().type)) {
        if (lookahead().type == TOKEN_TYPE::IDENT) {
            if (lookahead(1).type == TOKEN_TYPE::BRACKET_OPEN || lookahead(1).type == TOKEN_TYPE::ASSIGN) {
                parseStatement();
            } else {
                parseDeclaration();
            }
        }
        else if (isDeclaration(lookahead().type)) {
            parseDeclaration();
        } else {
            parseStatement();
        }
    }
}

void Parser::parseTypedef() {
    match(TOKEN_TYPE::TYPEDEF);
    parseType();
    match(TOKEN_TYPE::IDENT, PARSER_ERROR::MISSING_IDENTIFIER);
    
    if (lookahead().type == TOKEN_TYPE::BRACKET_OPEN) {
        match(TOKEN_TYPE::BRACKET_OPEN, PARSER_ERROR::MISSING_LBRACKET);
        parseEqualityExpression();
        match(TOKEN_TYPE::BRACKET_CLOSE, PARSER_ERROR::MISSING_RBRACKET);
    }

    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
}

void Parser::parseDeclaration() {
    parseType();
    parseVariableList();
    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
}

void Parser::parseType() {
    TOKEN_TYPE type = lookahead().type;

    if (type == TOKEN_TYPE::INT) {
        match(TOKEN_TYPE::INT);
    } else if (type == TOKEN_TYPE::SHORT) {
        match(TOKEN_TYPE::SHORT);
    } else if (type == TOKEN_TYPE::LONG) {
        match(TOKEN_TYPE::LONG);
    } else if (type == TOKEN_TYPE::CHAR) {
        match(TOKEN_TYPE::CHAR);
    } else {
        match(TOKEN_TYPE::IDENT, PARSER_ERROR::MISSING_TYPE_SPECIFIER);
    }
}

void Parser::parseVariableList() {
    while (true) {
        match(TOKEN_TYPE::IDENT, PARSER_ERROR::MISSING_IDENTIFIER);

        bool isArrayDeclaration = false;

        if (lookahead().type == TOKEN_TYPE::BRACKET_OPEN) {
            isArrayDeclaration = true;
            match(TOKEN_TYPE::BRACKET_OPEN);
            parseEqualityExpression();
            match(TOKEN_TYPE::BRACKET_CLOSE, PARSER_ERROR::MISSING_RBRACKET);
        }
        
        if (lookahead().type == TOKEN_TYPE::ASSIGN) {
            match(TOKEN_TYPE::ASSIGN);

            if (isArrayDeclaration) {
                match(TOKEN_TYPE::BRACE_OPEN, PARSER_ERROR::MISSING_LBRACE);
                
                // Empty init list --> 'type ident[expr] = {};
                if (lookahead().type == TOKEN_TYPE::BRACE_CLOSE) {
                    match(TOKEN_TYPE::BRACE_CLOSE);
                } else {
                    // Parsing values between {} --> '{expr, expr, ...}'
                    while (true) {
                        parseEqualityExpression();

                        if (lookahead().type == TOKEN_TYPE::COMMA) {
                            match(TOKEN_TYPE::COMMA);   
                        } else {
                            break;
                        }
                    }

                    match(TOKEN_TYPE::BRACE_CLOSE, PARSER_ERROR::MISSING_RBRACE);
                }
            } else {
                parseEqualityExpression();
            }
        }

        if (lookahead().type == TOKEN_TYPE::COMMA) {
            match(TOKEN_TYPE::COMMA);
        } else {
            break;
        }
    };
}

void Parser::parseStatement() {
    if (lookahead().type == TOKEN_TYPE::FOR) {
        parseForStatement();
    } else if (lookahead().type == TOKEN_TYPE::BRACE_OPEN) {
        match(TOKEN_TYPE::BRACE_OPEN);
        parseBlock();
        match(TOKEN_TYPE::BRACE_CLOSE, PARSER_ERROR::MISSING_RBRACE);
    } else if (lookahead().type == TOKEN_TYPE::IDENT) {
        parseAssignmentStatement();
        match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
    }  else {
        match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
    }
}

void Parser::parseForStatement() {
    match(TOKEN_TYPE::FOR);
    match(TOKEN_TYPE::PAR_OPEN, PARSER_ERROR::MISSING_LPAREN);

    if (lookahead().type == TOKEN_TYPE::IDENT) {
        parseAssignmentStatement();
    }

    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
    
    if (lookahead().type != TOKEN_TYPE::SEMICOLON) {
        parseEqualityExpression();
    }

    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);

    if (lookahead().type == TOKEN_TYPE::IDENT) {
        parseAssignmentStatement();
    }

    match(TOKEN_TYPE::PAR_CLOSE, PARSER_ERROR::MISSING_RPAREN);
    parseStatement();
}

void Parser::parseAssignmentStatement() {
    match(TOKEN_TYPE::IDENT);

    if (lookahead().type == TOKEN_TYPE::BRACKET_OPEN) {
        match(TOKEN_TYPE::BRACKET_OPEN);
        parseEqualityExpression();
        match(TOKEN_TYPE::BRACKET_CLOSE, PARSER_ERROR::MISSING_RBRACKET);
    }
 
    match(TOKEN_TYPE::ASSIGN, PARSER_ERROR::MISSING_ASSIGN);
    parseEqualityExpression();
}

void Parser::parseEqualityExpression() {
    parseComparisonExpression();

    while (lookahead().type == TOKEN_TYPE::EQ || lookahead().type == TOKEN_TYPE::NEQ) {
        match(lookahead().type); // Consume == or !=
        parseComparisonExpression();
    }
}

void Parser::parseComparisonExpression() {
    parseBitwiseShiftExpression();

    while (lookahead().type == TOKEN_TYPE::LT || lookahead().type == TOKEN_TYPE::LE ||
           lookahead().type == TOKEN_TYPE::GT || lookahead().type == TOKEN_TYPE::GE)
    {
        match(lookahead().type); // Consume <, <=, >= or >
        parseBitwiseShiftExpression();
    }
}

void Parser::parseBitwiseShiftExpression() {
    parseAdditiveExpression();

    while (lookahead().type == TOKEN_TYPE::BLS || lookahead().type == TOKEN_TYPE::BRS) {
        match(lookahead().type); // Consume << or >>
        parseAdditiveExpression();
    }
}

void Parser::parseAdditiveExpression() {
    parseMultiplicativeExpression();

    while (lookahead().type == TOKEN_TYPE::PLUS || lookahead().type == TOKEN_TYPE::MINUS) {
        match(lookahead().type); // Consume + or -
        parseMultiplicativeExpression();
    }
}

void Parser::parseMultiplicativeExpression() {
    parseUnaryExpression();

    while (lookahead().type == TOKEN_TYPE::MULT || lookahead().type == TOKEN_TYPE::DIV || lookahead().type == TOKEN_TYPE::MOD) {
        match(lookahead().type); // Consume *, / or %
        parseUnaryExpression();
    }
}

void Parser::parseUnaryExpression() {
    if (lookahead().type == TOKEN_TYPE::MINUS || lookahead().type == TOKEN_TYPE::PLUS) {
        match(lookahead().type); // Consume sign of a constant or of an identifier
    }

    if (lookahead().type == TOKEN_TYPE::PAR_OPEN) {
        match(TOKEN_TYPE::PAR_OPEN);
        parseEqualityExpression();
        match(TOKEN_TYPE::PAR_CLOSE, PARSER_ERROR::MISSING_LPAREN);
    } else if (isConstant(lookahead().type)) {
        match(lookahead().type);
    } else {
        match(TOKEN_TYPE::IDENT, PARSER_ERROR::INVALID_EXPRESSION);

        if (lookahead().type == TOKEN_TYPE::BRACKET_OPEN) {
            match(TOKEN_TYPE::BRACKET_OPEN);
            parseEqualityExpression();
            match(TOKEN_TYPE::BRACKET_CLOSE, PARSER_ERROR::MISSING_RBRACKET);
        }
    }
}

bool Parser::isConstant(TOKEN_TYPE type) const {
    return type == TOKEN_TYPE::CONST_DEC || type == TOKEN_TYPE::CONST_HEX ||
           type == TOKEN_TYPE::CONST_SYMB || type == TOKEN_TYPE::CONST_STR;
}

void Parser::error(PARSER_ERROR code, const Token& found) {
    std::string message;

    switch (code) {
        case PARSER_ERROR::MISSING_TYPE_SPECIFIER: {
            message = "expected type specifier";
            break;
        }
        case PARSER_ERROR::MISSING_LPAREN: {
            message = "expected '('";
            break;
        }
        case PARSER_ERROR::MISSING_RPAREN: {
            message = "expected ')'";
            break;
        }
        case PARSER_ERROR::MISSING_LBRACE: {
            message = "expected '{'";
            break;
        }
        case PARSER_ERROR::MISSING_RBRACE: {
            message = "expected '}'";
            break;
        }
        case PARSER_ERROR::MISSING_IDENTIFIER: {
            message = "expected identifier";
            break;
        }
        case PARSER_ERROR::MISSING_LBRACKET: {
            message = "expected '[";
            break;
        }
        case PARSER_ERROR::MISSING_RBRACKET: {
            message = "expected ']'";
            break;
        }
        case PARSER_ERROR::MISSING_SEMICOLON: {
            message = "expected ';'";
            break;
        }
        case PARSER_ERROR::MISSING_ASSIGN: {
            message = "expected '='";
            break;
        }
        case PARSER_ERROR::INVALID_EXPRESSION: {
            message = "expected expression";
            break;
        }
        
        default: break;
    }

    bool isLineFeedSkipped = lexer.isLineFeedSkipped();

    std::cerr << std::format(
        "{}:{}:{}: syntax error: {}\n",
        lexer.getFilePath(),
        isLineFeedSkipped ? m_previousLineEnd : found.m_lineStart,
        isLineFeedSkipped ? m_previousColumnEnd : found.m_columnStart,
        message
    );

    exit(EXIT_FAILURE);
}

void Parser::tokenDebug() const {
    Token token = lookahead();
    std::cout << std::format(
        "LS: {}, LE: {}, CS: {}, CE: {}\n",
        token.m_lineStart, token.m_lineEnd,
        token.m_columnStart, token.m_columnEnd
    );
}