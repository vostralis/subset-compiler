#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "symbol_table.hpp"
#include "visitor.hpp"

class Analyzer : public Visitor {
public:
    Analyzer(const std::string& path);
    
public:
    void analyze(ASTNode& root);

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

    int32_t evaluateConstantExpression(ExpressionNode*);
    bool isIntegerType(ASTNode::DataType type) const;
    void symbDebug(const std::string& name, Symbol* symbol) const;
    void error(const std::string& error, ASTNode* node) const;

private:
    SymbolTable m_symbolTable;
    std::string m_filePath;
};

#endif // ANALYZER_HPP