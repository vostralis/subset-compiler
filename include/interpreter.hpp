#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "symbol_table.hpp"
#include "visitor.hpp"

#include <iostream>
#include <format>

class Interpreter : public Visitor {
public:
    explicit Interpreter(const std::string& filepath, SymbolTable& symbolTable);

public:
    void interprete(ASTNode& root);

private:
    void visit(IdentifierNode&) override;
    void visit(ConstantNode&) override;
    void visit(BinaryOpNode&) override;
    void visit(ArrayIndexNode&) override;
    void visit(AssignmentNode&) override;
    void visit(EmptyStatementNode&) override;
    void visit(CompoundStatementNode&) override;
    void visit(ForNode&) override;
    void visit(VariableDeclNode&) override;
    void visit(ArrayDeclNode&) override;
    void visit(TypedefNode&) override;
    void visit(MainDeclNode&) override;
    void visit(ProgramNode&) override;

    void error(const std::string& error, ASTNode* node) const;
    void variantPrinter(const ValueVariant& val) const;
    void performAssignment(Symbol* target, const ValueVariant& rhs);
    int64_t getNumericValue(const ValueVariant& val) const;
    ValueVariant createValue(ASTNode::DataType type, int64_t val) const;

private:
    std::string m_filepath;
    SymbolTable& m_symbolTable;
    bool m_isInterpretationEnabled;
    ValueVariant m_lastExpressionValue;
};

#endif // INTERPRETER_HPP