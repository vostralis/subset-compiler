#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <vector>
#include <unordered_map>
#include <variant>

#include "ast.hpp"

using ValueVariant = std::variant<std::monostate, char, int, long, short>;

struct Symbol {
    ASTNode::DataType type;
    bool isArray = false;
    int32_t arraySize = -1;
    bool isTypedef = false;
    DeclarationNode *declarationNode;
    ValueVariant value;
    std::vector<ValueVariant> arrayValues;
};

using Scope = std::unordered_map<std::string, Symbol>;

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    bool isUniqueInCurrentScope(const std::string& name) const;
    bool declare(const std::string& name, Symbol&& symbol);
    Symbol* lookupSymbol(const std::string& name);

    void enterScope();
    void leaveScope();

private:
    std::vector<Scope> m_scopeStack;
    bool isMainDeclared;
};

#endif // SYMBOL_TABLE_HPP