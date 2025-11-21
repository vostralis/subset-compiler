#include "ast.hpp"


ASTNode::ASTNode(size_t m_lineStart, size_t m_column) :
    m_line(m_lineStart), m_column(m_column)
{}

ExpressionNode::ExpressionNode(size_t line, size_t column) : ASTNode(line, column) {}

StatementNode::StatementNode(size_t line, size_t column) : ASTNode(line, column) {}

DeclarationNode::DeclarationNode(size_t line, size_t column) : StatementNode(line, column) {}

IdentifierNode::IdentifierNode(size_t line, size_t column, const std::string& name) :
    ExpressionNode(line, column), name(name)
{}

std::string ASTNode::operatorToString(ASTNode::OperatorType op) {
    switch (op) {
        case ASTNode::OperatorType::ADD:  return "+";
        case ASTNode::OperatorType::SUB:  return "-";
        case ASTNode::OperatorType::MULT: return "*";
        case ASTNode::OperatorType::DIV:  return "\\";
        case ASTNode::OperatorType::MOD:  return "%";
        case ASTNode::OperatorType::EQ:   return "==";
        case ASTNode::OperatorType::NEQ:  return "!=";
        case ASTNode::OperatorType::LT:   return "<";
        case ASTNode::OperatorType::LE:   return "<=";
        case ASTNode::OperatorType::GT:   return ">";
        case ASTNode::OperatorType::GE:   return ">=";
        case ASTNode::OperatorType::BLS:  return "<<";
        case ASTNode::OperatorType::BRS:  return ">>";
        default: return "Unknown operation";
    };
}

std::string ASTNode::typeToString(ASTNode::DataType type) {
    switch (type) {
        case ASTNode::DataType::INT:     return "int";
        case ASTNode::DataType::SHORT:   return "short";
        case ASTNode::DataType::LONG:    return "long";
        case ASTNode::DataType::CHAR:    return "char";
        case ASTNode::DataType::UNKNOWN: return "custom";
        default: return "Unknown data type";
    }
}

void IdentifierNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string IdentifierNode::toString() const {
    return "Identifier: " + name;
}

ConstantNode::ConstantNode(size_t line, size_t column) : ExpressionNode(line, column) {}

void ConstantNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string ConstantNode::toString() const {
    std::string str = "Constant";
    
    switch (type) {
        case ConstantType::INT_10: {
            str += "(int10): " + value;
            break;
        }
        case ConstantType::INT_16: {
            str += "(int16): " + value;
            break;
        }
        case ConstantType::CHAR_LITERAL: {
            str += "(char): '" + value + '\'';
            break;
        }
        case ConstantType::STRING_LITERAL: {
            str += "(string): \"" + value + "\"";
            break;
        }
    }

    return str;
}

BinaryOpNode::BinaryOpNode(size_t line, size_t column) : ExpressionNode(line, column) {}

void BinaryOpNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string BinaryOpNode::toString() const {
    return "BinaryOp(" + operatorToString(op) + ")";
}

ArrayIndexNode::ArrayIndexNode(size_t line, size_t column) : ExpressionNode(line, column) {}

void ArrayIndexNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string ArrayIndexNode::toString() const {
    return "ArrayIndex";
}

AssignmentNode::AssignmentNode(size_t line, size_t column) : StatementNode(line, column) {}

void AssignmentNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string AssignmentNode::toString() const {
    return "Assignment(=)";
}

EmptyStatementNode::EmptyStatementNode(size_t line, size_t column) : StatementNode(line, column) {}

void EmptyStatementNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string EmptyStatementNode::toString() const {
    return "EmptyStatement(;)";
}

CompoundStatementNode::CompoundStatementNode() : StatementNode(0, 0) {}

void CompoundStatementNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string CompoundStatementNode::toString() const {
    return "CompoundStatement";
}

ForNode::ForNode(size_t line, size_t column) : StatementNode(line, column) {}

void ForNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string ForNode::toString() const {
    return "ForNode";
}

VariableDeclNode::VariableDeclNode(size_t line, size_t column) : DeclarationNode(line, column) {}

void VariableDeclNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string VariableDeclNode::toString() const {
    return "VariableDecl(" + typeToString(type) + ")";
}

ArrayDeclNode::ArrayDeclNode(size_t line, size_t column) : DeclarationNode(line, column) {}

void ArrayDeclNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string ArrayDeclNode::toString() const {
    return "ArrayDecl(" + typeToString(baseType) + ")";
}

TypedefNode::TypedefNode(size_t line, size_t column) : DeclarationNode(line, column) {}

void TypedefNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string TypedefNode::toString() const {
    return "Typedef; base type: " + typeToString(baseType) 
            + ", new typename: " + newTypeName->name;
}

MainDeclNode::MainDeclNode(size_t line, size_t column) : DeclarationNode(line, column) {}

void MainDeclNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string MainDeclNode::toString() const {
    return "MainFunction";
}

ProgramNode::ProgramNode() : ASTNode(0, 0) {}

void ProgramNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

std::string ProgramNode::toString() const {
    return "ProgramRoot";
}
