#include "analyzer.hpp"

#include <format>
#include <iostream>

Analyzer::Analyzer(const std::string& path) : m_filePath(path) {}

void Analyzer::analyze(ASTNode& root) {
    root.accept(*this);
}

// *
void Analyzer::visit(IdentifierNode& node) {
    Symbol *symbol = m_symbolTable.lookupSymbol(node.name);

    if (symbol == nullptr) {
        error("identifier usage before a declaration", &node);
    }

    if (symbol->isTypedef) {
        error("typename '" + node.name + "' was used as a variable name", &node);
    }

    if (symbol->isArray) {
        node.resolvedType = ASTNode::DataType::ARRAY;
    } else {
        node.resolvedType = symbol->type;
    }
}

// *
void Analyzer::visit(ConstantNode& node) {
    switch (node.type) {
        case ASTNode::ConstantType::INT_10:
        case ASTNode::ConstantType::INT_16:
            node.resolvedType = ASTNode::DataType::INT;
            break;
        case ASTNode::ConstantType::CHAR_LITERAL:
            node.resolvedType = ASTNode::DataType::CHAR;
            break;
        case ASTNode::ConstantType::STRING_LITERAL:
            node.resolvedType = ASTNode::DataType::ARRAY;
            break;
    }
}

// *
void Analyzer::visit(BinaryOpNode& node) {
    node.left->accept(*this);
    node.right->accept(*this);

    ASTNode::DataType leftType = node.left->resolvedType;
    ASTNode::DataType rightType = node.right->resolvedType;

    if (leftType == ASTNode::DataType::UNKNOWN || rightType == ASTNode::DataType::UNKNOWN) {
        node.resolvedType = ASTNode::DataType::UNKNOWN;
        return;
    }

    switch (node.op) {
        case ASTNode::OperatorType::ADD:
        case ASTNode::OperatorType::SUB:
        case ASTNode::OperatorType::MULT:
        case ASTNode::OperatorType::DIV:
        case ASTNode::OperatorType::MOD:
        case ASTNode::OperatorType::BLS:
        case ASTNode::OperatorType::BRS:
        {
            // Both operands must be integers
            if (!isIntegerType(leftType) || !isIntegerType(rightType)) {
                error("operands for arithmetic/shift operations must be integers", isIntegerType(leftType) ? node.right.get() : node.left.get());
            }
            
            // (long > int > short > char)
            node.resolvedType = (leftType > rightType) ? leftType : rightType;
            break;
        }

        case ASTNode::OperatorType::EQ:
        case ASTNode::OperatorType::NEQ:
        case ASTNode::OperatorType::LT:
        case ASTNode::OperatorType::LE:
        case ASTNode::OperatorType::GT:
        case ASTNode::OperatorType::GE:
        {
            // Both operands must be integers
            if (!isIntegerType(leftType) || !isIntegerType(rightType)) {
                error("operands for a comparison operation must be integers", isIntegerType(leftType) ? node.right.get() : node.left.get());
            }
            
            // Using int as a boolean
            node.resolvedType = ASTNode::DataType::INT;
            break;
        }
    }
}

// *
void Analyzer::visit(ArrayIndexNode& node) {
    node.identifier->accept(*this);
    node.indexExpression->accept(*this);

    Symbol *symbol = m_symbolTable.lookupSymbol(node.identifier->name);
    if (symbol == nullptr || !symbol->isArray) {
        error("attempt to index not an array", &node);
    }

    node.resolvedType = symbol->type;
}

// *
void Analyzer::visit(AssignmentNode& node) {
    node.left->accept(*this);
    node.right->accept(*this);

    bool isLValue = false;

    // Left member of a binary operation must be l-value
    if (IdentifierNode* ident = dynamic_cast<IdentifierNode*>(node.left.get())) {
        Symbol* symbol = m_symbolTable.lookupSymbol(ident->name);
        
        // Symbol is present and it's not an array
        if (symbol && !symbol->isArray) {
            isLValue = true;
        }
    } else if (dynamic_cast<ArrayIndexNode*>(node.left.get())) {
        isLValue = true;
    }

    if (!isLValue) {
        error("left operand of an assignment operator must be a l-value", &node);
    }
}

// *
void Analyzer::visit([[maybe_unused]] EmptyStatementNode& node) {}

// *
void Analyzer::visit(CompoundStatementNode& node) {
    m_symbolTable.enterScope();

    for (auto& statement : node.statements) {
        statement->accept(*this);
    }

    m_symbolTable.leaveScope();
}

// *
void Analyzer::visit(ForNode& node) {
    m_symbolTable.enterScope();

    if (node.init) node.init->accept(*this);
    if (node.condition) {
        node.condition->accept(*this);
        if (!isIntegerType(node.condition->resolvedType)) {
            error("the loop condition must be resolvable to a boolean (integer) value", node.condition.get());
        }
    }
    if (node.increment) node.increment->accept(*this);
    if (node.body) node.body->accept(*this);

    m_symbolTable.leaveScope();
}

// *
void Analyzer::visit(VariableDeclNode& node) {
    std::string name = node.identifier->name;

    if (!m_symbolTable.isUniqueInCurrentScope(name)) {
        error("redeclaration of '" + name + "'", &node);
    }

    if (Symbol* symbol = m_symbolTable.lookupSymbol(name)) {
        if (symbol->isTypedef) error("typename '" + name + "' was used as a variable name", &node);
    }

    ASTNode::DataType finalType = node.type;
    Symbol newSymbol;

    if (node.typedefName) {
        std::string typeName = node.typedefName->name;
        Symbol *symbol = m_symbolTable.lookupSymbol(typeName);
        
        if (symbol == nullptr || !symbol->isTypedef) {
            error("usage of an undefined type '" + typeName + "'", node.typedefName.get());
        }

        newSymbol.isArray = symbol->isArray;

        if (newSymbol.isArray) {
            newSymbol.arraySize = symbol->arraySize;
        }

        finalType = symbol->type;
    } else {
        newSymbol.isArray = false;
    }

    if (node.initExpression) {
        node.initExpression->accept(*this);
    }

    newSymbol.type = finalType;
    newSymbol.isTypedef = false;
    newSymbol.declarationNode = &node;

    m_symbolTable.declare(name, std::move(newSymbol));
}

// *
void Analyzer::visit(ArrayDeclNode& node) {
    std::string name = node.identifier->name;

    if (!m_symbolTable.isUniqueInCurrentScope(name)) {
        error("redeclaration of '" + name + "'", &node);
    }

    if (Symbol* symbol = m_symbolTable.lookupSymbol(name)) {
        if (symbol->isTypedef) error("typename '" + name + "' was used as a variable name", &node);
    }

    Symbol newSymbol;
    newSymbol.isArray = true;
    newSymbol.declarationNode = &node;

    int32_t calculatedSize = -1;

    if (node.typedefName) {
        std::string typeName = node.typedefName->name;
        Symbol *symbol = m_symbolTable.lookupSymbol(typeName);
        
        if (symbol == nullptr || !symbol->isTypedef) {
            error("usage of an undefined type '" + typeName + "'", node.typedefName.get());
        }

        if (symbol->isArray && node.sizeExpression) {
            error("underlying type is already an array", &node);
        }

        calculatedSize = symbol->arraySize;
        newSymbol.type = symbol->type;
    }

    // Array's size is explicitly specified
    if (node.sizeExpression) {
        node.sizeExpression->accept(*this);

        calculatedSize = evaluateConstantExpression(node.sizeExpression.get());
        if (calculatedSize <= 0) {
            error("the array size must be greater that 0", &node);
        }
    }

    if (node.stringLiteralInit) {
        if (node.baseType != ASTNode::DataType::CHAR) {
            error("an array of type other than ‘char’ can't be initialized with a string", &node);
        }

        int32_t stringLength = node.stringLiteralInit->value.length() + 1;

        // Array's length isn't specified
        if (calculatedSize == -1) {
            calculatedSize = stringLength;
        } else if (calculatedSize < stringLength) {
            error(
                "an array of size " + std::to_string(calculatedSize) + 
                " is too small for initialization with a string of size " + std::to_string(stringLength),
                &node
            );
        }
    } else if (!node.braceListInit.empty()) {
        int listSize = node.braceListInit.size();

        // Array's length isn't specified
        if (calculatedSize == -1) {
            calculatedSize = listSize;
        } else if (calculatedSize < listSize) {
            error(
                "too many initializers for an array of size " + std::to_string(calculatedSize),
                node.braceListInit[0].get()
            );
        }

        for (auto& expression : node.braceListInit) {
            expression->accept(*this);
        }
    }

    if (calculatedSize == -1) {
        error("failed to determine the size of the array '" + name + "'", &node);
    }

    // std::cout << name << "[" << calculatedSize << "]\n";

    newSymbol.type = node.baseType;
    newSymbol.arraySize = calculatedSize;

    m_symbolTable.declare(name, std::move(newSymbol));
}

// *
void Analyzer::visit(TypedefNode& node) {
    std::string name = node.newTypeName->name;

    if (!m_symbolTable.isUniqueInCurrentScope(name)) {
         error("redeclaration of '" + name + "'", &node);
    }

    Symbol newSymbol;
    newSymbol.isTypedef = true;
    newSymbol.declarationNode = &node;
    
    if (node.baseTypeCustom) {
        Symbol *symbol = m_symbolTable.lookupSymbol(node.baseTypeCustom->name);

        if (symbol == nullptr) {
           error("identifier usage before a declaration", node.baseTypeCustom.get());
        }

        newSymbol.type = symbol->type;
        newSymbol.isArray = symbol->isArray;

        if (symbol->isArray) {
            if (node.arraySizeExpression) {
                error("underlying type is already an array", &node);
            }

            newSymbol.arraySize = symbol->arraySize;
        }

    } else {
        newSymbol.type = node.baseType;
        if (node.arraySizeExpression) {
            newSymbol.isArray = true;
            try {
                newSymbol.arraySize = evaluateConstantExpression(node.arraySizeExpression.get());
            } catch (const std::exception& e) {
                error("array size in typedef expression must be a consant value", node.arraySizeExpression.get());
            }
        } else {
            newSymbol.isArray = false;
        }
    }

    m_symbolTable.declare(name, std::move(newSymbol));
}

// *
void Analyzer::visit(MainDeclNode& node) {
    if (m_symbolTable.lookupSymbol("main") != nullptr) {
        error("main function is already declared", &node);
    }

    Symbol newSymbol;
    newSymbol.isArray = false;
    newSymbol.isTypedef = false;
    newSymbol.declarationNode = &node;
    newSymbol.type = ASTNode::DataType::INT;

    m_symbolTable.declare("main", std::move(newSymbol));

    node.body->accept(*this);
}

// *
void Analyzer::visit(ProgramNode& node) {
    for (auto& decl : node.declarations) {
        decl->accept(*this);
    }
}

int32_t Analyzer::evaluateConstantExpression(ExpressionNode* node) {
    if (ConstantNode* constNode = dynamic_cast<ConstantNode*>(node)) {
        try {
            if (constNode->type == ASTNode::ConstantType::INT_10) {
                return std::stoi(constNode->value);
            }

            if (constNode->type == ASTNode::ConstantType::INT_16) {
                return std::stoi(constNode->value, nullptr, 0);
            }

            if (constNode->type == ASTNode::ConstantType::CHAR_LITERAL) {
                return static_cast<int>(constNode->value[0]);
            }
        } catch (const std::exception& e) {
            throw std::logic_error(e.what());
        }
    }

    if (BinaryOpNode* binOpNode = dynamic_cast<BinaryOpNode*>(node)) {
        int32_t left = evaluateConstantExpression(binOpNode->left.get());
        int32_t right = evaluateConstantExpression(binOpNode->right.get());

        switch (binOpNode->op) {
            case ASTNode::OperatorType::ADD:  return left + right;
            case ASTNode::OperatorType::SUB:  return left - right;
            case ASTNode::OperatorType::MULT: return left * right;
            case ASTNode::OperatorType::DIV: {
                if (right == 0) throw std::logic_error("Division by zero");
                return left / right;
            }
            case ASTNode::OperatorType::MOD:  return left % right;
            case ASTNode::OperatorType::EQ:   return left == right ? 1 : 0;
            case ASTNode::OperatorType::NEQ:  return left != right ? 1 : 0;
            case ASTNode::OperatorType::LT:   return left < right ? 1 : 0;
            case ASTNode::OperatorType::LE:   return left <= right ? 1 : 0;
            case ASTNode::OperatorType::GT:   return left > right ? 1 : 0;
            case ASTNode::OperatorType::GE:   return left >= right ? 1 : 0;
            default:
                // !
                break;
        }
    }

    throw std::runtime_error("An expression is not a constant at the compile time");
}

bool Analyzer::isIntegerType(ASTNode::DataType type) const {
    return type == ASTNode::DataType::INT || type == ASTNode::DataType::SHORT ||
           type == ASTNode::DataType::LONG || type == ASTNode::DataType::CHAR;
}

void Analyzer::symbDebug(const std::string& name, Symbol* symbol) const {
    std::cout << name << ", type: ";
    switch (symbol->type) {
        case ASTNode::DataType::CHAR: std::cout << "char"; break;
        case ASTNode::DataType::INT:  std::cout << "int"; break;
        case ASTNode::DataType::SHORT: std::cout << "short"; break;
        case ASTNode::DataType::LONG: std::cout << "long"; break;
        case ASTNode::DataType::ARRAY: std::cout << "array"; break;
        case ASTNode::DataType::CUSTOM: std::cout << "custom"; break;
        case ASTNode::DataType::UNKNOWN: std::cout << "unknown"; break;
    }
    
    std::cout << ", isArray: " << std::boolalpha << symbol->isArray;
    if (symbol->isArray) std::cout << ", arraySize: " << symbol->arraySize;
    std::cout << ", isTypedef: " << std::boolalpha << symbol->isTypedef;
    std::cout << std::endl;
}

void Analyzer::error(const std::string& error, ASTNode* node) const {
    std::cerr << std::format(
        "{}:{}:{}: semantic error: {}\n", m_filePath, node->m_line, node->m_column, error
    );

    exit(EXIT_FAILURE);
}