#include "interpreter.hpp"
#include "analyzer.hpp"

Interpreter::Interpreter(const std::string& filepath, SymbolTable& symbolTable) : 
    m_filepath(filepath),
    m_symbolTable(symbolTable)    
{}

void Interpreter::interprete(ASTNode& root) {
    root.accept(*this);
}

void Interpreter::visit(IdentifierNode& node) {
    if (std::holds_alternative<std::monostate>(node.symbolPtr->value)) {
        error("Usage of uninitialized variable + " + node.name, &node);
    }

    m_lastExpressionValue = std::variant(node.symbolPtr->value);

    if (std::holds_alternative<long>(m_lastExpressionValue)) {
        std::cout << "long\n";
    }
}

void Interpreter::visit(ConstantNode& node) {
    // The Analyzer already resolved node.resolvedType. 
    // We strictly use that to ensure the variant holds the correct alternative.
    switch (node.resolvedType) {
        case ASTNode::DataType::CHAR:
            m_lastExpressionValue = static_cast<char>(node.value[0]);
            break;
        case ASTNode::DataType::INT:
        case ASTNode::DataType::SHORT:
        case ASTNode::DataType::LONG:
            // Parse based on radix provided in node.type
            if (node.type == ASTNode::ConstantType::INT_16) {
                long long val = std::stoll(node.value, nullptr, 16);
                m_lastExpressionValue = createValue(node.resolvedType, val);
            } else {
                long long val = std::stoll(node.value);
                m_lastExpressionValue = createValue(node.resolvedType, val);
            }
            break;
        default:
             m_lastExpressionValue = std::monostate{};
    }
}

void Interpreter::visit(BinaryOpNode& node) {
    node.left->accept(*this);
    ValueVariant lhs = m_lastExpressionValue;
    node.right->accept(*this);
    ValueVariant rhs = m_lastExpressionValue;

    m_lastExpressionValue = std::visit(
        [&](auto&& l_val, auto&& r_val) -> ValueVariant {
            using T1 = std::decay_t<decltype(l_val)>;
            using T2 = std::decay_t<decltype(r_val)>;

            if constexpr (std::is_same_v<T1, std::monostate> || std::is_same_v<T2, std::monostate>) {
                error("usage of uninitialzed variable", node.left.get());
            } else {
                
                long long v1 = static_cast<long long>(l_val);
                long long v2 = static_cast<long long>(r_val);

                switch (node.op) {
                    case ASTNode::OperatorType::ADD:  return v1 + v2;
                    case ASTNode::OperatorType::SUB:  return v1 - v2;
                    case ASTNode::OperatorType::MULT: return v1 * v2;
                    case ASTNode::OperatorType::DIV:  
                        if (v2 % 2 == 0) error("division by 0", node.right.get());
                        return v1 / v2;
                    case ASTNode::OperatorType::MOD:  
                        if (v2 % 2 == 0) error("division by 0", node.right.get());
                        return v1 % v2;

                    case ASTNode::OperatorType::BLS: return v1 << v2;
                    case ASTNode::OperatorType::BRS: return v1 >> v2;
                    
                    case ASTNode::OperatorType::EQ:  return (v1 == v2) ? 1 : 0;
                    case ASTNode::OperatorType::NEQ: return (v1 != v2) ? 1 : 0;
                    case ASTNode::OperatorType::LT:  return (v1 < v2) ? 1 : 0;
                    case ASTNode::OperatorType::LE:  return (v1 <= v2) ? 1 : 0;
                    case ASTNode::OperatorType::GT:  return (v1 > v2) ? 1 : 0;
                    case ASTNode::OperatorType::GE:  return (v1 >= v2) ? 1 : 0;
                }
            }                
            return std::monostate{};
        },
        lhs, rhs
    );

            
    ASTNode::DataType resultType = node.resolvedType;
    std::cout << static_cast<int>(resultType) << std::endl;

    std::visit([resultType](auto& value){
        if constexpr (!std::is_same_v<std::decay_t<decltype(value)>, std::monostate>) {
            switch (resultType) {
                case ASTNode::DataType::INT:   value = static_cast<int>(value); break;
                case ASTNode::DataType::SHORT: value = static_cast<short>(value); break;
                case ASTNode::DataType::LONG:  value = static_cast<long>(value); break;
                case ASTNode::DataType::CHAR:  value = static_cast<char>(value); break;
                default: break;
            }
        }
    }, m_lastExpressionValue);
}

void Interpreter::visit([[maybe_unused]]ArrayIndexNode& node) {
    return;
}

void Interpreter::visit(AssignmentNode& node) {
    node.right->accept(*this);
    ValueVariant rhs = m_lastExpressionValue;

    if (IdentifierNode *ident = dynamic_cast<IdentifierNode*>(node.left.get())) {
        ident->symbolPtr->value = rhs;
        std::cout << "[Assignment]: " << ident->name << " = ";
        variantPrinter(rhs);
        std::cout << std::endl;
    }
}

void Interpreter::visit([[maybe_unused]]EmptyStatementNode& node) {
    return;
}

void Interpreter::visit(CompoundStatementNode& node) {
    for (auto& statement : node.statements) {
        statement->accept(*this);
    }
}

void Interpreter::visit([[maybe_unused]]ForNode& node) {
    return;
}

void Interpreter::visit(VariableDeclNode& node) {
    if (node.initExpression) {
        node.initExpression->accept(*this);
        ValueVariant initValue = m_lastExpressionValue;
        Symbol *symbol = node.identifier->symbolPtr;
        symbol->value = initValue;
        std::cout << "[Declaration]: " << node.identifier->name << " = ";
        variantPrinter(symbol->value);
        std::cout << std::endl;
    }
}

void Interpreter::visit(ArrayDeclNode& node) {
    Symbol *symbol = node.identifier->symbolPtr;
    std::cout << symbol->arraySize << std::endl;
}

void Interpreter::visit([[maybe_unused]]TypedefNode& node) {
    return;
}

void Interpreter::visit(MainDeclNode& node) {
    node.body->accept(*this);
}

void Interpreter::visit(ProgramNode& node) {
    for (auto& declaration : node.declarations) {
        declaration->accept(*this);
    }
}

void Interpreter::error(const std::string& error, ASTNode* node) const {
    std::cerr << std::format(
        "{}:{}:{}: semantic error: {}\n", m_filepath, node->m_line, node->m_column, error
    );

    exit(EXIT_FAILURE);
}

void Interpreter::variantPrinter(const ValueVariant& val) const {
    if (std::holds_alternative<long>(val)) {
        std::cout << "(long) " << std::get<long>(val);
    } else if (std::holds_alternative<int>(val)) {
        std::cout << "(int) " << std::get<int>(val);
    } else if (std::holds_alternative<short>(val)) {
        std::cout << "(short) " << std::get<short>(val);
    }  else if (std::holds_alternative<char>(val)) {
        char ch = std::get<char>(val);
        std::cout << "(char) " << "'" << ch << "' (ASCII: " << static_cast<int>(ch) << ")";
    } else if (std::holds_alternative<std::monostate>(val)) {
        std::cout << "?";
    }
}

void Interpreter::performAssignment(Symbol* target, const ValueVariant& rhs) {
    ASTNode::DataType targetType = target->type;

    
}

// Extract a numeric value from the variant as long long
int64_t Interpreter::getNumericValue(const ValueVariant& val) const {
    if (auto p = std::get_if<char>(&val)) return static_cast<long long>(*p);
    if (auto p = std::get_if<short>(&val)) return static_cast<long long>(*p);
    if (auto p = std::get_if<int>(&val)) return static_cast<long long>(*p);
    if (auto p = std::get_if<long>(&val)) return static_cast<long long>(*p);
    throw std::runtime_error("Runtime Error: Accessing uninitialized or invalid value.");
}

// Create a variant of the specific AST type from a raw numeric value
ValueVariant Interpreter::createValue(ASTNode::DataType type, int64_t val) const {
    switch (type) {
        case ASTNode::DataType::CHAR:  return static_cast<char>(val);
        case ASTNode::DataType::SHORT: return static_cast<short>(val);
        case ASTNode::DataType::INT:   return static_cast<int>(val);
        case ASTNode::DataType::LONG:  return static_cast<long>(val);
        default:
            throw std::runtime_error("Runtime Error: Cannot create value for unknown/custom type.");
    }
}