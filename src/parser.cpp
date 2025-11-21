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

Token Parser::consume() {
    Token token = lookahead();
    m_previousLineEnd = token.m_lineEnd;
    m_previousColumnEnd = token.m_columnEnd;

    m_lookaheadBuffer[m_bufferPos] = lexer.getNextToken();
    m_bufferPos = (m_bufferPos + 1) % BUFFER_SIZE;

    return token;
}

void Parser::match(TOKEN_TYPE expected, PARSER_ERROR mismatchCode) {
    Token found = lookahead();

    if (found.type == expected) {
        //lookahead().print();
        consumedToken = consume();
    } else {
        error(mismatchCode, found);
    }
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto programNode = std::make_unique<ProgramNode>();

    while (isDescriptionStart(lookahead().type)) {
        if (lookahead().type == TOKEN_TYPE::INT && lookahead(1).type == TOKEN_TYPE::MAIN) {
            auto mainNode = parseMainFunction();
            programNode->declarations.push_back(std::move(mainNode));
        } else if (lookahead().type == TOKEN_TYPE::TYPEDEF) {
            auto typedefNode = parseTypedef();
            programNode->declarations.push_back(std::move(typedefNode));
        } else {
            auto declaration = parseDeclaration();
            for (auto& decl : declaration) {
                programNode->declarations.push_back(std::move(decl));
            }
        }
    }

    match(TOKEN_TYPE::END);

    return programNode;
}

bool Parser::isDescriptionStart(TOKEN_TYPE type) const {
    return type == TOKEN_TYPE::TYPEDEF || type == TOKEN_TYPE::INT ||
           type == TOKEN_TYPE::SHORT || type == TOKEN_TYPE::LONG ||
           type == TOKEN_TYPE::CHAR || type == TOKEN_TYPE::IDENT;
}

std::unique_ptr<DeclarationNode> Parser::parseMainFunction() {
    
    match(TOKEN_TYPE::INT, PARSER_ERROR::MISSING_TYPE_SPECIFIER);
    auto mainNode = std::make_unique<MainDeclNode>(consumedToken.m_lineStart, consumedToken.m_columnStart);
    match(TOKEN_TYPE::MAIN, PARSER_ERROR::UNEXPECTED_TOKEN);
    match(TOKEN_TYPE::LPAREN, PARSER_ERROR::MISSING_LPAREN);
    match(TOKEN_TYPE::RPAREN, PARSER_ERROR::MISSING_RPAREN);
    match(TOKEN_TYPE::LBRACE, PARSER_ERROR::MISSING_LBRACE);
    mainNode->body = parseCompoundStatement();
    match(TOKEN_TYPE::RBRACE, PARSER_ERROR::MISSING_RBRACE);
    
    return mainNode;
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
        case TOKEN_TYPE::LBRACE:
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

std::unique_ptr<CompoundStatementNode> Parser::parseCompoundStatement() {
    auto compoundNode = std::make_unique<CompoundStatementNode>();

    while (isStatementOrDeclarationStart(lookahead().type)) {
        if (lookahead().type == TOKEN_TYPE::IDENT) {
            if (lookahead(1).type == TOKEN_TYPE::LBRACKET || lookahead(1).type == TOKEN_TYPE::ASSIGN) {
                auto statementNode = parseStatement();
                compoundNode->statements.push_back(std::move(statementNode));
            } else {
                auto declaration = parseDeclaration();
                for (auto& decl : declaration) {
                    compoundNode->statements.push_back(std::move(decl));
                }
            }
        }
        else if (isDeclaration(lookahead().type)) {
            auto declaration = parseDeclaration();
            for (auto& decl : declaration) {
                compoundNode->statements.push_back(std::move(decl));
            }
        } else {
            auto statementNode = parseStatement();
            compoundNode->statements.push_back(std::move(statementNode));
        }
    }

    return compoundNode;
}

std::unique_ptr<TypedefNode> Parser::parseTypedef() {
    auto typedefNode = std::make_unique<TypedefNode>(lookahead().m_lineStart, lookahead().m_columnStart);
    match(TOKEN_TYPE::TYPEDEF);

    ParsedType underlyingType = parseTypeSpecifier();

    if (underlyingType.typeName) {
        typedefNode->baseTypeCustom = std::move(underlyingType.typeName);
    } else {
        typedefNode->baseType = underlyingType.baseType;
    }

    match(TOKEN_TYPE::IDENT, PARSER_ERROR::MISSING_IDENTIFIER);
    typedefNode->newTypeName = std::make_unique<IdentifierNode>(
        consumedToken.m_lineStart, consumedToken.m_columnStart, std::get<std::string>(consumedToken.value)
    );

    if (lookahead().type == TOKEN_TYPE::LBRACKET) {
        match(TOKEN_TYPE::LBRACKET, PARSER_ERROR::MISSING_LBRACKET);
        typedefNode->arraySizeExpression = parseEqualityExpression();
        match(TOKEN_TYPE::RBRACKET, PARSER_ERROR::MISSING_RBRACKET);
    }

    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);

    return typedefNode;
}

std::vector<std::unique_ptr<DeclarationNode>> Parser::parseDeclaration() {
    auto typeInfo = parseTypeSpecifier();
    auto declarations = parseVariableList(typeInfo);
    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
    return declarations;
}

ParsedType Parser::parseTypeSpecifier() {
    ParsedType parsed;
    match(lookahead().type);

    switch (consumedToken.type) {
        case TOKEN_TYPE::INT:   parsed.baseType = ASTNode::DataType::INT; break;
        case TOKEN_TYPE::SHORT: parsed.baseType = ASTNode::DataType::SHORT; break;
        case TOKEN_TYPE::LONG:  parsed.baseType = ASTNode::DataType::LONG; break;
        case TOKEN_TYPE::CHAR:  parsed.baseType = ASTNode::DataType::CHAR; break;
        case TOKEN_TYPE::IDENT: {
            parsed.typeName = std::make_unique<IdentifierNode>(
                consumedToken.m_lineStart, consumedToken.m_columnStart, std::get<std::string>(consumedToken.value)
            );
            break;
        }

        default: break;
    }

    return parsed;
}

std::vector<std::unique_ptr<DeclarationNode>> Parser::parseVariableList(const ParsedType& typeInfo) {
    std::vector<std::unique_ptr<DeclarationNode>> declarations;

    while (true) {
        auto declarationNode = parseSingleVariableDeclaration(typeInfo);
        declarations.push_back(std::move(declarationNode));

        if (lookahead().type == TOKEN_TYPE::COMMA) {
            match(TOKEN_TYPE::COMMA);
        } else {
            break;
        }
    }

    return declarations;
}

std::unique_ptr<DeclarationNode> Parser::parseSingleVariableDeclaration(const ParsedType& typeInfo) {
    match(TOKEN_TYPE::IDENT, PARSER_ERROR::MISSING_IDENTIFIER);
    auto identifier = std::make_unique<IdentifierNode>(
        consumedToken.m_lineStart, consumedToken.m_columnStart, std::get<std::string>(consumedToken.value)
    );

    if (lookahead().type == TOKEN_TYPE::LBRACKET) {
        auto arrayNode = std::make_unique<ArrayDeclNode>(identifier->m_line, identifier->m_column);

        if (typeInfo.typeName) {
            arrayNode->typedefName = std::make_unique<IdentifierNode>(
                identifier->m_line, identifier->m_column, typeInfo.typeName->name
            );
        } else {
            arrayNode->baseType = typeInfo.baseType;
        }
        
        arrayNode->identifier = std::move(identifier);

        match(TOKEN_TYPE::LBRACKET);
        // Array's length is specified
        if (lookahead().type != TOKEN_TYPE::RBRACKET) {
            arrayNode->sizeExpression = parseEqualityExpression();
        }
        match(TOKEN_TYPE::RBRACKET, PARSER_ERROR::MISSING_RBRACKET);

        if (lookahead().type != TOKEN_TYPE::ASSIGN) {
            return arrayNode;
        }
        
        match(TOKEN_TYPE::ASSIGN);

        if (lookahead().type == TOKEN_TYPE::LBRACE) {
            match(TOKEN_TYPE::LBRACE);

            // Empty init list --> 'type ident[expr] = {};
            if (lookahead().type == TOKEN_TYPE::RBRACE) {
                match(TOKEN_TYPE::RBRACE);
            } else {
                // Parsing values between {} --> '{expr, expr, ...}'
                while (true) {
                    auto expression = parseEqualityExpression();
                    arrayNode->braceListInit.push_back(std::move(expression));

                    if (lookahead().type != TOKEN_TYPE::COMMA) {
                        break;  
                    }

                    match(TOKEN_TYPE::COMMA);
                }

                match(TOKEN_TYPE::RBRACE, PARSER_ERROR::MISSING_RBRACE);
            }

        } else {
            // char ident[expr] = "string";
            match(TOKEN_TYPE::CONST_STR, PARSER_ERROR::INVALID_EXPRESSION);
            auto stringLiteral = std::make_unique<ConstantNode>(
                consumedToken.m_lineStart, consumedToken.m_columnStart
            );
            stringLiteral->type = ASTNode::ConstantType::STRING_LITERAL;
            stringLiteral->value = std::get<std::string>(consumedToken.value);
            arrayNode->stringLiteralInit = std::move(stringLiteral);
        }

        return arrayNode;
    } else {
        auto variableNode = std::make_unique<VariableDeclNode>(
            identifier->m_line, identifier->m_column
        );

        if (typeInfo.typeName) {
            variableNode->typedefName = std::make_unique<IdentifierNode>(
                typeInfo.typeName->m_line, typeInfo.typeName->m_column, typeInfo.typeName->name
            );
        } else {
            variableNode->type = typeInfo.baseType;
        }

        variableNode->identifier = std::move(identifier);

        if (lookahead().type == TOKEN_TYPE::ASSIGN) {  
            match(TOKEN_TYPE::ASSIGN);
            variableNode->initExpression = parseEqualityExpression();
        }

        return variableNode;
    }
}

std::unique_ptr<StatementNode> Parser::parseStatement() {
    if (lookahead().type == TOKEN_TYPE::FOR) {
        return parseForStatement();
    } else if (lookahead().type == TOKEN_TYPE::LBRACE) {
        match(TOKEN_TYPE::LBRACE);
        auto compoundStatementNode = parseCompoundStatement();
        match(TOKEN_TYPE::RBRACE, PARSER_ERROR::MISSING_RBRACE);
        return compoundStatementNode;
    } else if (lookahead().type == TOKEN_TYPE::IDENT) {
        auto assignmentNode = parseAssignmentStatement();
        match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
        return assignmentNode;
    }  else {
        match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
        return std::make_unique<EmptyStatementNode>(consumedToken.m_lineStart, consumedToken.m_columnStart);
    }
}

std::unique_ptr<ForNode> Parser::parseForStatement() {
    match(TOKEN_TYPE::FOR);
    auto forNode = std::make_unique<ForNode>(consumedToken.m_lineStart, consumedToken.m_columnStart);

    match(TOKEN_TYPE::LPAREN, PARSER_ERROR::MISSING_LPAREN);

    if (lookahead().type == TOKEN_TYPE::IDENT) {
        forNode->init = parseAssignmentStatement();
    }

    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);
    
    if (lookahead().type != TOKEN_TYPE::SEMICOLON) {
        forNode->condition = parseEqualityExpression();
    }

    match(TOKEN_TYPE::SEMICOLON, PARSER_ERROR::MISSING_SEMICOLON);

    if (lookahead().type == TOKEN_TYPE::IDENT) {
        forNode->increment = parseAssignmentStatement();
    }

    match(TOKEN_TYPE::RPAREN, PARSER_ERROR::MISSING_RPAREN);
    forNode->body = parseStatement();

    return forNode;
}

std::unique_ptr<AssignmentNode> Parser::parseAssignmentStatement() {
    std::unique_ptr<AssignmentNode> assignmentNode;
    
    if (lookahead(1).type == TOKEN_TYPE::LBRACKET) {
        match(TOKEN_TYPE::IDENT);
        assignmentNode = std::make_unique<AssignmentNode>(
            consumedToken.m_lineStart, consumedToken.m_columnStart
        );
        auto arrayIndexNode = std::make_unique<ArrayIndexNode>(
            consumedToken.m_lineStart, consumedToken.m_columnStart
        );
        arrayIndexNode->identifier = std::make_unique<IdentifierNode>(
            consumedToken.m_lineStart, consumedToken.m_columnStart,
            std::get<std::string>(consumedToken.value)
        );
        match(TOKEN_TYPE::LBRACKET);
        arrayIndexNode->indexExpression = parseEqualityExpression();
        match(TOKEN_TYPE::RBRACKET, PARSER_ERROR::MISSING_RBRACKET);
        assignmentNode->left = std::move(arrayIndexNode);
    } else {
        match(TOKEN_TYPE::IDENT);
        assignmentNode = std::make_unique<AssignmentNode>(
            consumedToken.m_lineStart, consumedToken.m_columnStart
        );
        assignmentNode->left = std::make_unique<IdentifierNode>(
            consumedToken.m_lineStart, consumedToken.m_columnStart,
            std::get<std::string>(consumedToken.value)
        );
    }
 
    match(TOKEN_TYPE::ASSIGN, PARSER_ERROR::MISSING_ASSIGN);
    assignmentNode->right = parseEqualityExpression();

    return assignmentNode;
}

std::unique_ptr<ExpressionNode> Parser::parseEqualityExpression() {
    auto leftNode = parseComparisonExpression();
    
    while (lookahead().type == TOKEN_TYPE::EQ || lookahead().type == TOKEN_TYPE::NEQ) {
        TOKEN_TYPE opToken = lookahead().type;
        match(lookahead().type); // Consume == or !=
        auto rightNode = parseComparisonExpression();
        auto binaryNode = std::make_unique<BinaryOpNode>(leftNode->m_line, leftNode->m_column);

        if (opToken == TOKEN_TYPE::EQ) {
            binaryNode->op = ASTNode::OperatorType::EQ;
        } else {
            binaryNode->op = ASTNode::OperatorType::NEQ;
        }

        binaryNode->left = std::move(leftNode);
        binaryNode->right = std::move(rightNode);
        leftNode = std::move(binaryNode);
    }

    return leftNode;
}

std::unique_ptr<ExpressionNode> Parser::parseComparisonExpression() {
    auto leftNode = parseBitwiseShiftExpression();
    
    while (lookahead().type == TOKEN_TYPE::LT || lookahead().type == TOKEN_TYPE::LE ||
           lookahead().type == TOKEN_TYPE::GT || lookahead().type == TOKEN_TYPE::GE)
    {
        TOKEN_TYPE opToken = lookahead().type;
        match(lookahead().type); // Consume <, <=, >= or >
        auto rightNode = parseBitwiseShiftExpression();
        auto binaryNode = std::make_unique<BinaryOpNode>(leftNode->m_line, leftNode->m_column);

        if (opToken == TOKEN_TYPE::LT) {
            binaryNode->op = ASTNode::OperatorType::LT;
        } else if (opToken == TOKEN_TYPE::LE) {
            binaryNode->op = ASTNode::OperatorType::LE;
        } else if (opToken == TOKEN_TYPE::GT) {
            binaryNode->op = ASTNode::OperatorType::GT;
        } else {
            binaryNode->op = ASTNode::OperatorType::GE;
        }

        binaryNode->left = std::move(leftNode);
        binaryNode->right = std::move(rightNode);
        leftNode = std::move(binaryNode);
    }

    return leftNode;
}

std::unique_ptr<ExpressionNode> Parser::parseBitwiseShiftExpression() {
    auto leftNode = parseAdditiveExpression();
    
    while (lookahead().type == TOKEN_TYPE::BLS || lookahead().type == TOKEN_TYPE::BRS) {
        TOKEN_TYPE opToken = lookahead().type;
        match(lookahead().type); // Consume << or >>
        auto rightNode = parseAdditiveExpression();
        auto binaryNode = std::make_unique<BinaryOpNode>(leftNode->m_line, leftNode->m_column);

        if (opToken == TOKEN_TYPE::BLS) {
            binaryNode->op = ASTNode::OperatorType::BLS;
        } else {
            binaryNode->op = ASTNode::OperatorType::BRS;
        }

        binaryNode->left = std::move(leftNode);
        binaryNode->right = std::move(rightNode);
        leftNode = std::move(binaryNode);
    }

    return leftNode;
}

std::unique_ptr<ExpressionNode> Parser::parseAdditiveExpression() {
    auto leftNode = parseMultiplicativeExpression();

    while (lookahead().type == TOKEN_TYPE::PLUS || lookahead().type == TOKEN_TYPE::MINUS) {
        TOKEN_TYPE opToken = lookahead().type;
        match(lookahead().type); // Consume + or -
        auto rightNode = parseMultiplicativeExpression();
        auto binaryNode = std::make_unique<BinaryOpNode>(leftNode->m_line, leftNode->m_column);

        if (opToken == TOKEN_TYPE::PLUS) {
            binaryNode->op = ASTNode::OperatorType::ADD;
        } else {
            binaryNode->op = ASTNode::OperatorType::SUB;
        }

        binaryNode->left = std::move(leftNode);
        binaryNode->right = std::move(rightNode);
        leftNode = std::move(binaryNode);
    }

    return leftNode;
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicativeExpression() {
    auto leftNode = parseUnaryExpression();
    
    while (lookahead().type == TOKEN_TYPE::MULT || lookahead().type == TOKEN_TYPE::DIV || lookahead().type == TOKEN_TYPE::MOD) {
        TOKEN_TYPE opToken = lookahead().type;
        match(lookahead().type); // Consume *, / or %
        auto rightNode = parseUnaryExpression();
        auto binaryNode = std::make_unique<BinaryOpNode>(leftNode->m_line, leftNode->m_column);

        if (opToken == TOKEN_TYPE::MULT) {
            binaryNode->op = ASTNode::OperatorType::MULT;
        } else if (opToken == TOKEN_TYPE::DIV) {
            binaryNode->op = ASTNode::OperatorType::DIV;
        } else {
            binaryNode->op = ASTNode::OperatorType::MOD;
        }

        binaryNode->left = std::move(leftNode);
        binaryNode->right = std::move(rightNode);
        leftNode = std::move(binaryNode);
    }

    return leftNode;
}

std::unique_ptr<ExpressionNode> Parser::parseUnaryExpression() {
    bool isNegative = false;
    if (lookahead().type == TOKEN_TYPE::MINUS || lookahead().type == TOKEN_TYPE::PLUS) {
        if (lookahead().type == TOKEN_TYPE::MINUS) {
            isNegative = true;
        }

        match(lookahead().type); // Consume sign of a constant or of an identifier
    }

    if (lookahead().type == TOKEN_TYPE::LPAREN) {
        match(TOKEN_TYPE::LPAREN);
        auto expressionNode = parseEqualityExpression();
        match(TOKEN_TYPE::RPAREN, PARSER_ERROR::MISSING_LPAREN);
        return expressionNode;
    } else if (isConstant(lookahead().type)) {
        match(lookahead().type);
        auto constantNode = std::make_unique<ConstantNode>(
            consumedToken.m_lineStart, consumedToken.m_columnStart
        );

        switch (consumedToken.type) {
            case TOKEN_TYPE::CONST_DEC: {
                constantNode->type = ASTNode::ConstantType::INT_10;        
                break;
            }
            case TOKEN_TYPE::CONST_HEX: {
                constantNode->type = ASTNode::ConstantType::INT_16;
                break;
            }
            case TOKEN_TYPE::CONST_SYMB: {
                constantNode->type = ASTNode::ConstantType::CHAR_LITERAL;
                break;
            }
            case TOKEN_TYPE::CONST_STR: {
                constantNode->type = ASTNode::ConstantType::STRING_LITERAL;
                break;
            }
            default: break;
        }

        if (consumedToken.type == TOKEN_TYPE::CONST_SYMB) {
            constantNode->value = std::get<char>(consumedToken.value);
        } else {
            constantNode->value = std::get<std::string>(consumedToken.value);
            if (isNegative) {
                constantNode->value = "-" + constantNode->value;
            }
        }

        return constantNode;
    } else {
        // ident[expr]
        if (lookahead(1).type == TOKEN_TYPE::LBRACKET) {
            match(TOKEN_TYPE::IDENT, PARSER_ERROR::INVALID_EXPRESSION);
            auto arrayIndexNode = std::make_unique<ArrayIndexNode>(
                consumedToken.m_lineStart, consumedToken.m_columnStart
            );
            arrayIndexNode->identifier = std::make_unique<IdentifierNode>(
                consumedToken.m_lineStart, consumedToken.m_columnStart,
                std::get<std::string>(consumedToken.value)
            );
            match(TOKEN_TYPE::LBRACKET);
            arrayIndexNode->indexExpression = parseEqualityExpression();
            match(TOKEN_TYPE::RBRACKET, PARSER_ERROR::MISSING_RBRACKET);
            return arrayIndexNode;
        } else {
            match(TOKEN_TYPE::IDENT, PARSER_ERROR::INVALID_EXPRESSION);
            return std::make_unique<IdentifierNode>(
                consumedToken.m_lineStart, consumedToken.m_columnStart,
                std::get<std::string>(consumedToken.value)
            );
        }
    }
}

bool Parser::isConstant(TOKEN_TYPE type) const {
    return type == TOKEN_TYPE::CONST_DEC || type == TOKEN_TYPE::CONST_HEX || type == TOKEN_TYPE::CONST_SYMB;
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