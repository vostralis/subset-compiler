#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <memory>
#include <vector>

#include "visitor.hpp"

struct ASTNode {
    ASTNode(size_t line, size_t column);
    virtual ~ASTNode() = default;
    virtual void accept(Visitor& visitor) = 0;
    virtual std::string toString() const = 0;

    // Types of data
    enum class DataType {
        CHAR, SHORT, INT, LONG, ARRAY, CUSTOM, UNKNOWN
    };

    // Types of constants
    enum class ConstantType {
        INT_10, INT_16, CHAR_LITERAL, STRING_LITERAL
    };

    // Types of operators
    enum class OperatorType {
        // Arithmetic
        ADD, SUB, MULT, DIV, MOD,
        // Comparison
        EQ, NEQ, LT, LE, GT, GE,
        // Shifts
        BLS, BRS
    };

    static std::string operatorToString(ASTNode::OperatorType op);
    static std::string typeToString(ASTNode::DataType type);

    size_t m_line, m_column;
};

// 
struct ParsedType {
    using DataType = ASTNode::DataType;

    DataType baseType = DataType::UNKNOWN;
    std::unique_ptr<IdentifierNode> typeName = nullptr;
};

// Base node for expressions
struct ExpressionNode : ASTNode {
    ExpressionNode(size_t line, size_t column);

    DataType resolvedType;
};

// Base node for statements
struct StatementNode : ASTNode {
    StatementNode(size_t line, size_t column);
};

// Base node for declarations
struct DeclarationNode : StatementNode {
    DeclarationNode(size_t line, size_t column);
};

// Node for identifiers
struct IdentifierNode : ExpressionNode {
    IdentifierNode(size_t line, size_t column, const std::string& name);

    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::string name;
};

// Node for constants
struct ConstantNode : ExpressionNode {
    ConstantNode(size_t line, size_t column);
   
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::string value;
    ConstantType type;
};

// Node for binary statements
struct BinaryOpNode : ExpressionNode {
    BinaryOpNode(size_t line, size_t column);

    void accept(Visitor& visitor) override;
    std::string toString() const override;
    
    OperatorType op;
    std::unique_ptr<ExpressionNode> left, right;
};

// Node for an array indexing
struct ArrayIndexNode : ExpressionNode {
    ArrayIndexNode(size_t line, size_t column);
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<ExpressionNode> indexExpression;
};

// Node for assignment statements
struct AssignmentNode : StatementNode {
    AssignmentNode(size_t line, size_t column);

    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
};

// Node for empty statements
struct EmptyStatementNode : StatementNode {
    EmptyStatementNode(size_t line, size_t column);

    void accept(Visitor& visitor) override;
    std::string toString() const override;
};

// Node for compound statements
struct CompoundStatementNode : StatementNode {
    CompoundStatementNode();
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::vector<std::unique_ptr<StatementNode>> statements;
};

// Node for for-statements
struct ForNode : StatementNode {
    ForNode(size_t line, size_t column);
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::unique_ptr<AssignmentNode> init = nullptr;
    std::unique_ptr<ExpressionNode> condition = nullptr;
    std::unique_ptr<AssignmentNode> increment = nullptr;
    std::unique_ptr<StatementNode> body;
};

// Node for variable declaration statements
struct VariableDeclNode : DeclarationNode {
    VariableDeclNode(size_t line, size_t column);
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    // Either a base type, either a typedef-name 
    DataType type = DataType::UNKNOWN;
    std::unique_ptr<IdentifierNode> typedefName = nullptr;

    std::unique_ptr<IdentifierNode> identifier = nullptr;
    std::unique_ptr<ExpressionNode> initExpression = nullptr;
};

// Node for array declaration statements
struct ArrayDeclNode : DeclarationNode {
    ArrayDeclNode(size_t line, size_t column);
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    // Either a base type, either a typedef-name 
    DataType baseType;
    std::unique_ptr<IdentifierNode> typedefName = nullptr;

    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<ExpressionNode> sizeExpression;

    std::vector<std::unique_ptr<ExpressionNode>> braceListInit;
    std::unique_ptr<ConstantNode> stringLiteralInit;
};

// Node for typedef-statements
struct TypedefNode : DeclarationNode {
    TypedefNode(size_t line, size_t column);
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    DataType baseType;
    std::unique_ptr<IdentifierNode> baseTypeCustom = nullptr;

    std::unique_ptr<IdentifierNode> newTypeName;
    std::unique_ptr<ExpressionNode> arraySizeExpression;
};

// Node for the main function declaration
struct MainDeclNode : DeclarationNode {
    MainDeclNode(size_t line, size_t column);
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::unique_ptr<IdentifierNode> name;
    std::unique_ptr<CompoundStatementNode> body;
};

// Root node of the AST
struct ProgramNode : ASTNode {
    ProgramNode();
    
    void accept(Visitor& visitor) override;
    std::string toString() const override;

    std::vector<std::unique_ptr<DeclarationNode>> declarations;
};

#endif // AST_HPP