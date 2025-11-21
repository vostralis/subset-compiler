#include "symbol_table.hpp"

#include <iostream>

SymbolTable::SymbolTable() : 
    isMainDeclared(false) 
{
    enterScope(); // Create the global scope
}

SymbolTable::~SymbolTable() {
    leaveScope(); // Exit the global scope
}

void SymbolTable::enterScope() {
    m_scopeStack.emplace_back();
}

void SymbolTable::leaveScope() {
    // To prevent exiting from a global scope    
    if (m_scopeStack.size() > 1) {
        m_scopeStack.pop_back();
    }
}

bool SymbolTable::declare(const std::string& name, Symbol&& symbol) {
    if (m_scopeStack.empty()) return false;

    m_scopeStack.back().insert(std::make_pair(name, symbol));
    return true;
}

bool SymbolTable::isUniqueInCurrentScope(const std::string& name) const {
    if (m_scopeStack.empty()) return false;

    const auto& scope = m_scopeStack.back();
    return scope.find(name) == scope.end();
}

Symbol* SymbolTable::lookupSymbol(const std::string& name) {
    if (m_scopeStack.empty()) return nullptr;

    for (auto it = m_scopeStack.rbegin(); it != m_scopeStack.rend(); ++it) {
        auto found = it->find(name);

        if (found != it->end()) {
            return &found->second;
        }
    }

    return nullptr;
}