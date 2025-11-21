#include "ast_printer.hpp"

#include <iostream>

void ASTPrinter::printNode(const std::string& info) {
    for (size_t i = 0; i < m_indentationLevel; ++i) {
        std::cout << "  ";
    }

    std::cout << "- " + info << std::endl;
}

void ASTPrinter::indent() { m_indentationLevel++; }

void ASTPrinter::unindent() { m_indentationLevel--; }

void ASTPrinter::print(ASTNode& root) {
    root.accept(*this);
}

void ASTPrinter::visit(IdentifierNode& node) {
    printNode(node.toString());   
}

void ASTPrinter::visit(ConstantNode& node) {
    printNode(node.toString());
}

void ASTPrinter::visit([[maybe_unused]]BinaryOpNode& node) {
    return;
    // printNode(node.toString());
    // indent();
    
    // node.left->accept(*this);
    // node.right->accept(*this);

    // unindent();
}

void ASTPrinter::visit([[maybe_unused]]ArrayIndexNode& node) {
    return;
    // printNode(node.toString());
    // indent();

    // node.identifier->accept(*this);
    // node.indexExpression->accept(*this);

    // unindent();
}

void ASTPrinter::visit([[maybe_unused]]AssignmentNode& node) {
    return;    
    // printNode(node.toString());
    // indent();

    // node.left->accept(*this);
    // node.right->accept(*this);
    
    // unindent();
}

void ASTPrinter::visit([[maybe_unused]]EmptyStatementNode& node) {
    return;
}

void ASTPrinter::visit(CompoundStatementNode& node) {
    printNode(node.toString());
    indent();

    for (auto& statement : node.statements) {
        statement->accept(*this);
    }

    unindent();
}

void ASTPrinter::visit(ForNode& node) {
    printNode(node.toString());
    indent();

    if (node.init) node.init->accept(*this);
    if (node.condition) node.condition->accept(*this);
    if (node.increment) node.increment->accept(*this);

    node.body->accept(*this);

    unindent();
}

void ASTPrinter::visit(VariableDeclNode& node) {
    std::string s = node.identifier->toString() + "; type: " + ASTNode::typeToString(node.type);
    printNode(s);
}

void ASTPrinter::visit(ArrayDeclNode& node) {
    std::string s = node.identifier->toString() + "; type: ";

    if (node.stringLiteralInit) {
        s += "string";
    } else {
        s += ASTNode::typeToString(node.baseType) + "[]";
    }

    printNode(s);
}

void ASTPrinter::visit(TypedefNode& node) {
    printNode(node.toString());
}

void ASTPrinter::visit(MainDeclNode& node) {
    printNode(node.toString());
    indent();

    node.body->accept(*this);

    unindent();
}

void ASTPrinter::visit(ProgramNode& node) {
    printNode(node.toString());
    indent();

    for (auto& decl : node.declarations) {
        decl->accept(*this);
    }

    unindent();
}
