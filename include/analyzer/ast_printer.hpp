#ifndef AST_PRINTER_HPP
#define AST_PRINTER_HPP

#include "visitor.hpp"
#include "ast.hpp"

class ASTPrinter : public Visitor {
public:
    void print(ASTNode& root);

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

    void printNode(const std::string& info);
    void indent();
    void unindent();

private:
    size_t m_indentationLevel = 0;
};

#endif // AST_PRINTER_HPP