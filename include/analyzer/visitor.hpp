#ifndef VISITOR_HPP
#define VISITOR_HPP

struct ASTNode;
struct IdentifierNode;
struct ConstantNode;
struct BinaryOpNode;
struct ArrayIndexNode;
struct AssignmentNode;
struct EmptyStatementNode;
struct CompoundStatementNode;
struct ForNode;
struct VariableDeclNode;
struct ArrayDeclNode;
struct TypedefNode;
struct MainDeclNode;
struct ProgramNode;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void visit(IdentifierNode&) = 0;
    virtual void visit(ConstantNode&) = 0;
    virtual void visit(BinaryOpNode&) = 0;
    virtual void visit(ArrayIndexNode&) = 0;
    virtual void visit(AssignmentNode&) = 0;
    virtual void visit(EmptyStatementNode&) = 0;
    virtual void visit(CompoundStatementNode&) = 0;
    virtual void visit(ForNode&) = 0;
    virtual void visit(VariableDeclNode&) = 0;
    virtual void visit(ArrayDeclNode&) = 0;
    virtual void visit(TypedefNode&) = 0;
    virtual void visit(MainDeclNode&) = 0;
    virtual void visit(ProgramNode&) = 0;
};

#endif // VISITOR_HPP