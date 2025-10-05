#include "lexer.hpp"

#include <iostream>
#include <format>

Lexer::Lexer() = default;

Lexer::~Lexer() {
    closeStream();
}

void Lexer::openStream() {
    m_inputStream.open(path, std::ios_base::in);

    if (!m_inputStream.is_open()) {
        throw std::runtime_error("Couldn't open file: " + path);
    }

    // Initial fill
    refillBuffer();

    // Reset line and column numbers
    m_line = m_column = 1;
}

void Lexer::closeStream() {
    if (!m_inputStream.is_open()) {
        m_inputStream.close();
    };
}

bool Lexer::refillBuffer() {
    // Read up to BUFFER_SIZE characters into the buffer
    m_inputStream.read(m_buffer.data(), m_buffer.size());

    // Get the actual number of characters read
    m_validSize = static_cast<size_t>(m_inputStream.gcount());

    // Reset the read pointer
    m_currIndex = 0;

    // If 0 characters were read, we reached the EOF
    return m_validSize > 0;
}

std::vector<Token> Lexer::parse(const std::string& path) {
    this->path = path;
    openStream();

    std::vector<Token> tokens;
    bool is_eof = false;

    while (!is_eof) {
        Token currentToken = getNextToken();

        switch (currentToken.type) {
            case TOKEN_TYPE::ERROR: {
                is_eof = true;
            }
            case TOKEN_TYPE::END: {
                is_eof = true;
            }
            default: {
                tokens.emplace_back(currentToken);
                break;
            }
        }
    }
    
    closeStream();
    
    return tokens;
}

Token Lexer::getNextToken() {
    char c = skipWhitespacesAndComments();

    // Symbolic constant: ' + symbol + '
    if (c == '\'') { 
        return parseSymbolicConstant();
    }
    // String constant: " + symbol sequence + "
    if (c == '\"') {
        return parseStringConstant();
    }

    // If token starts with letter or underscore, it's either a keyword, either an identificator
    if (std::isalpha(c) || c == '_') {
        return parseIdentifier(c);       
    }

    // If token starts with digit, it's either a decimal constant, either a hex constant
    if (std::isdigit(c)) {
        return parseNumericConstant(c);
    }

    m_column++; // Increment column counter

    switch (c) {
        case '\0': return Token(TOKEN_TYPE::END);
        case ',': return Token(TOKEN_TYPE::COMMA);
        case ';': return Token(TOKEN_TYPE::SEMICOLON);
        case '(': return Token(TOKEN_TYPE::PAR_OPEN);
        case ')': return Token(TOKEN_TYPE::PAR_CLOSE);
        case '{': return Token(TOKEN_TYPE::BRACE_OPEN);
        case '}': return Token(TOKEN_TYPE::BRACE_CLOSE);
        case '[': return Token(TOKEN_TYPE::BRACKET_OPEN);
        case ']': return Token(TOKEN_TYPE::BRACKET_CLOSE);
        case '<': {
            char next = getNextChar();

            // Bitwise left shift <<
            if (next == '<') {
                m_column++;
                return Token(TOKEN_TYPE::BLS);
            }
            // Operator <=
            else if (next == '=') {
                m_column++;
                return Token(TOKEN_TYPE::LE); 
            }
            // Operator <
            else {
                returnCharToBuffer(next);
                return Token(TOKEN_TYPE::LT);
            }
        }
        case '>': {
            char next = getNextChar();

            // Bitwise right shift
            if (next == '>') {
                m_column++;
                return Token(TOKEN_TYPE::BRS);
            }
            // Operator >=
            else if (next == '=') {
                m_column++;
                return Token(TOKEN_TYPE::GE); 
            }
            // Operator >
            else {
                returnCharToBuffer(next); 
                return Token(TOKEN_TYPE::GT);
            }
        }
        case '=': {
            char next = getNextChar();
            m_column++;
            
            // Operator =
            if (next == '=') return Token(TOKEN_TYPE::EQ);
            // Operator ==
            else {
                returnCharToBuffer(next);
                m_column--;
                return Token(TOKEN_TYPE::ASSIGN);
            }
        }
        case '!': {
            char next = getNextChar();

            // Operator !=
            if (next == '=') {
                m_column++; 
                return Token(TOKEN_TYPE::NEQ);
            }
            else {
                returnCharToBuffer(next);
                return Token(TOKEN_TYPE::ERROR, error("Invalid lexeme."));
            }
        }
        case '+': return Token(TOKEN_TYPE::PLUS);
        case '-': return Token(TOKEN_TYPE::MINUS);
        case '*': return Token(TOKEN_TYPE::MULT);
        case '/': return Token(TOKEN_TYPE::DIV);
        case '%': return Token(TOKEN_TYPE::MOD);
        default: m_column--; return Token(TOKEN_TYPE::ERROR, error("Invalid character."));
    }
}

char Lexer::getNextChar() {
    // First of all we check if there's character saved in pushback cell
    if (m_pushback.has_value()) {
        char c = m_pushback.value();
        m_pushback.reset();
        return c;
    }

    // If read pointer is at the end of a buffer, we have to refill it from the stream
    if (m_currIndex >= m_validSize) {
        if (!refillBuffer()) return '\0'; // End of a file
    }

    // Otherwise just return next character in the buffer
    return m_buffer[m_currIndex++];
}

void Lexer::returnCharToBuffer(char c) {
    if (m_currIndex == 0) {
        // If we have to return a character to a buffer, but it either was just refilled, either we're at the very start
        // of a program, we have to save this character
        m_pushback = c;
    } else {
        // Buffer wasn't refilled, so we can just decrement read pointer
        m_currIndex--;
    }
}

char Lexer::skipWhitespacesAndComments() {
    while (true) {
        char c = getNextChar();

        // Skip whitespace
        switch (c) {
            case '\n': {
                m_line++; // Increment the line counter when '\n' is encountered
                m_column = 1;
                continue;
            }
            case '\t': {
                m_column += 4;
                continue;
            }
            case ' ': {
                m_column++;
                continue;
            }
            default: break;
        }

        // Skip comment
        if (c == '/') {
            // If we have 2 subsequent '/', then comment starts
            // else return character to a buffer
            char next = getNextChar();
            if (next == '/') {
                while ((c = getNextChar()) != '\0' && c != '\n');
                m_line++;
                m_column = 1;
                continue;
            } else {
                returnCharToBuffer(next);
            }
        }

        return c;
    }
}

Token Lexer::lookupKeyword(std::string& lexeme) {
    m_column += lexeme.length();

    if (lexeme == "main")
        return Token(TOKEN_TYPE::MAIN);
    if (lexeme == "int")
        return Token(TOKEN_TYPE::INT);
    if (lexeme == "short")
        return Token(TOKEN_TYPE::SHORT);
    if (lexeme == "long")
        return Token(TOKEN_TYPE::LONG);
    if (lexeme == "char")
        return Token(TOKEN_TYPE::CHAR);
    if (lexeme == "typedef")
        return Token(TOKEN_TYPE::TYPEDEF);
    if (lexeme == "for")
        return Token(TOKEN_TYPE::FOR);

    return Token(TOKEN_TYPE::IDENT, std::move(lexeme));
}

Token Lexer::parseSymbolicConstant() {
    m_column++; // '\''
    char c = getNextChar(), next;

    // Symbolic constant must have a symbol between quotes
    if (c == '\'') {
        return Token(TOKEN_TYPE::ERROR, error("Symbolic constant can't be empty."));
    }
    
    if (c == '\\') {
        m_column++;
        c = getNextChar();
        next = getNextChar();

        if (next == '\'') {
            switch (c) {
                case 'n':  m_column += 2; return Token(TOKEN_TYPE::CONST_SYMB, '\n');
                case 't':  m_column += 2; return Token(TOKEN_TYPE::CONST_SYMB, '\t');
                case '\\': m_column += 2; return Token(TOKEN_TYPE::CONST_SYMB, '\\');
                case '\'': m_column += 2; return Token(TOKEN_TYPE::CONST_SYMB, '\'');
                default:   return Token(TOKEN_TYPE::ERROR, error("Invalid escape sequence."));
            }
        } else {
            return Token(TOKEN_TYPE::ERROR, error("Symbolic constant was never closed."));
        }
    } else {
        next = getNextChar();
    
        if (next == '\'') {
            m_column += 2; // Character + '\''
            return Token(TOKEN_TYPE::CONST_SYMB, c);
        } else {
            // Got more that 1 character in symbolic constant
            return Token(
                TOKEN_TYPE::ERROR,
                error("Symbolic constant can't contain more that 1 symbol.")
            );
        }
    }
}

Token Lexer::parseStringConstant() {
    m_column++; // '\"'
    std::string lexeme;
    char c = getNextChar();
    
    // Quote is closed immediately - empty string
    if (c == '\"') {
        m_column++;
        return Token(TOKEN_TYPE::CONST_STR, "");
    }

    while (c != '\"' && c != '\0') {
        if (c == '\\') {
            char escaped = getNextChar();
            switch (escaped) {
                case 'n':  lexeme += '\n'; m_column += 2; break;
                case 't':  lexeme += '\t'; m_column += 2; break;
                case '\\': lexeme += '\\'; m_column += 2; break;
                case '\"': lexeme += '\"'; m_column += 2; break;
                default:   return Token(TOKEN_TYPE::ERROR, error("Invalid escape sequence."));
            }
        } else {
            m_column++;
            lexeme += c;
        }

        c = getNextChar();
    }

    // Quote ("") was never closed
    if (c == '\0') {
        return Token(TOKEN_TYPE::ERROR, error("String constant was never closed"));
    }

    m_column++; // '\"'
    return Token(TOKEN_TYPE::CONST_STR, std::move(lexeme));
}

Token Lexer::parseIdentifier(char firstCharacter) {
    std::string lexeme;
    lexeme += firstCharacter;

    char next = getNextChar();

    while (std::isalpha(next) || std::isdigit(next) || next == '_') {
        lexeme += next;

        // Limit the identifier length
        if (lexeme.length() > MAX_IDENTIFIER_LENGTH) {
            return Token(
                TOKEN_TYPE::ERROR, 
                std::format("The length of an identifier must not exceed {} characters.", MAX_IDENTIFIER_LENGTH)
            );
        }

        next = getNextChar();
    }

    returnCharToBuffer(next);

    // Check if token is a keyword or an identifier
    return lookupKeyword(lexeme); 
}

Token Lexer::parseNumericConstant(char firstDigit) {
    std::string lexeme;
    lexeme += firstDigit;

    char c = getNextChar();

    if (firstDigit == '0' && std::tolower(c) == 'x') {
        // Hex constant starts with 0x or 0X
        do {
            lexeme += c;

            // 2147483647 = 0x7FFFFFFF, so we can at least discard constants
            // whose length is greater than 10 (including '0x' prefix)
            if (lexeme.length() > 10) {
                return Token(TOKEN_TYPE::ERROR, error("Hex constant is too long."));
            }

            c = getNextChar();
        } while (std::isxdigit(static_cast<unsigned char>(c)));

        // Invalid: "0x" with no digits after
        if (std::tolower(lexeme.back()) == 'x') {
            return Token(TOKEN_TYPE::ERROR, error("Invalid hex constant."));
        }

        returnCharToBuffer(c);
        m_column += lexeme.length();

        return Token(TOKEN_TYPE::CONST_HEX, std::move(lexeme));
    } else {
        // Decimal constant
        while (std::isdigit(c)) {
            lexeme += c;

            // len(2147483647) = 10, so we can at least discard constants
            // whose length is greater than 10
            if (lexeme.length() > 10) {
                return Token(TOKEN_TYPE::ERROR, error("Decimal constant is too long."));
            }

            c = getNextChar();
        }

        returnCharToBuffer(c);
        m_column += lexeme.length();
        
        return Token(TOKEN_TYPE::CONST_DEC, std::move(lexeme));
    }
}

std::string Lexer::error(std::string&& error) const {
    return std::format("{}:{}:{}: {}", path, m_line, m_column, error);
}