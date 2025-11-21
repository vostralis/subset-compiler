#include "lexer.hpp"

#include <iostream>
#include <format>

Lexer::Lexer(std::string_view path) : 
    m_path(path),
    m_isLineFeedSkipped(false)
{
    openStream();
}

Lexer::~Lexer() {
    closeStream();
}

void Lexer::openStream() {
    m_inputStream.open(m_path, std::ios_base::in);

    if (!m_inputStream.is_open()) {
        throw std::runtime_error("Couldn't open file: " + m_path);
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

Token Lexer::getNextToken() {
    if (!m_inputStream.is_open())  {
        return Token(TOKEN_TYPE::END, m_line, m_line, m_column, m_column);
    }

    char c = skipWhitespacesAndComments();
    if (c == '\0') {
        closeStream();
        return Token(TOKEN_TYPE::END, m_line, m_line, m_column, m_column);
    }

    size_t lineStart = m_line;
    size_t columnStart = m_column - 1;
    
    // Symbolic constant: ' + symbol + '
    if (c == '\'') { 
        return parseSymbolicConstant(lineStart, columnStart);
    }
    // String constant: " + symbol sequence + "
    if (c == '\"') {
        return parseStringConstant(lineStart, columnStart);
    }

    // If token starts with letter or underscore, it's either a keyword, either an identificator
    if (std::isalpha(c) || c == '_') {
        return parseIdentifier(c, lineStart, columnStart);       
    }

    // If token starts with digit, it's either a decimal constant, either a hex constant
    if (std::isdigit(c)) {
        return parseNumericConstant(c, lineStart, columnStart);
    }

    switch (c) {
        case ',': return Token(TOKEN_TYPE::COMMA, lineStart, m_line, columnStart, m_column);
        case ';': return Token(TOKEN_TYPE::SEMICOLON, lineStart, m_line, columnStart, m_column);
        case '(': return Token(TOKEN_TYPE::LPAREN, lineStart, m_line, columnStart, m_column);
        case ')': return Token(TOKEN_TYPE::RPAREN, lineStart, m_line, columnStart, m_column);
        case '{': return Token(TOKEN_TYPE::LBRACE, lineStart, m_line, columnStart, m_column);
        case '}': return Token(TOKEN_TYPE::RBRACE, lineStart, m_line, columnStart, m_column);
        case '[': return Token(TOKEN_TYPE::LBRACKET, lineStart, m_line, columnStart, m_column);
        case ']': return Token(TOKEN_TYPE::RBRACKET, lineStart, m_line, columnStart, m_column);
        case '<': {
            char next = getNextChar();

            // Bitwise left shift <<
            if (next == '<') return Token(TOKEN_TYPE::BLS, lineStart, m_line, columnStart, m_column);
            // Operator <=
            else if (next == '=') return Token(TOKEN_TYPE::LE, lineStart, m_line, columnStart, m_column); 
            // Operator <
            else {
                returnCharToBuffer(next);
                return Token(TOKEN_TYPE::LT, lineStart, m_line, columnStart, m_column);
            }
        }
        case '>': {
            char next = getNextChar();

            // Bitwise right shift
            if (next == '>') return Token(TOKEN_TYPE::BRS, lineStart, m_line, columnStart, m_column);
            // Operator >=
            else if (next == '=') return Token(TOKEN_TYPE::GE, lineStart, m_line, columnStart, m_column); 
            // Operator >
            else {
                returnCharToBuffer(next); 
                return Token(TOKEN_TYPE::GT, lineStart, m_line, columnStart, m_column);
            }
        }
        case '=': {
            char next = getNextChar();
            
            // Operator =
            if (next == '=') return Token(TOKEN_TYPE::EQ, lineStart, m_line, columnStart, m_column);
            // Operator ==
            else {
                returnCharToBuffer(next);
                return Token(TOKEN_TYPE::ASSIGN, lineStart, m_line, columnStart, m_column);
            }
        }
        case '!': {
            char next = getNextChar();

            // Operator !=
            if (next == '=') return Token(TOKEN_TYPE::NEQ, lineStart, m_line, columnStart, m_column);
            else {
                returnCharToBuffer(next);
                return Token(TOKEN_TYPE::ERROR, error("Invalid lexeme."), lineStart, m_line, columnStart, m_column);
            }
        }
        case '+': return Token(TOKEN_TYPE::PLUS, lineStart, m_line, columnStart, m_column);
        case '-': return Token(TOKEN_TYPE::MINUS, lineStart, m_line, columnStart, m_column);
        case '*': return Token(TOKEN_TYPE::MULT, lineStart, m_line, columnStart, m_column);
        case '/': return Token(TOKEN_TYPE::DIV, lineStart, m_line, columnStart, m_column);
        case '%': return Token(TOKEN_TYPE::MOD, lineStart, m_line, columnStart, m_column);
        default: return Token(TOKEN_TYPE::ERROR, error("Invalid character."), lineStart, m_line, columnStart, m_column);
    }
}

char Lexer::getNextChar() {
    char c;

    // First of all we check if there's character saved in pushback cell
    if (m_pushback.has_value()) {
        c = m_pushback.value();
        m_pushback.reset();
    } else {
        // If read pointer is at the end of a buffer, we have to refill it from the stream
        if (m_currIndex >= m_validSize && !refillBuffer())
            return '\0';
        // Otherwise just return next character in the buffer
        c = m_buffer[m_currIndex++];
    }

    // Tracking current line and column numbers
    m_prevColumn = m_column;
    switch (c) {
        case '\n': {
            m_line++;
            m_column = 1;
            break;
        }
        case '\t': {
            m_column += 4;
            break;
        }
        default: {
            m_column++;
            break;
        }
    }

    return c;
}

void Lexer::returnCharToBuffer(char c) {
    // Tracking current line and column numbers 
    switch (c) {
        case '\n': {
            m_line--;
            m_column = m_prevColumn;
            break;
        }
        case '\t': {
            m_column -= 4;
            break;
        }
        default: {
            m_column--;
            break;
        }
    }

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

        if (c == '\n' && !m_isLineFeedSkipped) m_isLineFeedSkipped = true;

        // Skip whitespace
        if (c == '\n' || c == '\t' || c == ' ') continue;
        
        // Skip comment
        if (c == '/') {
            // If we have 2 subsequent '/', then comment starts
            // else return character to a buffer
            char next = getNextChar();
            if (next == '/') {
                while ((c = getNextChar()) != '\0' && c != '\n');
                continue;
            } else {
                returnCharToBuffer(next);
            }
        }

        return c;
    }
}

Token Lexer::lookupKeyword(std::string& lexeme, size_t lineStart, size_t columnStart) {
    TOKEN_TYPE type;

    if (lexeme == "main")         type = TOKEN_TYPE::MAIN;
    else if (lexeme == "int")     type = TOKEN_TYPE::INT;
    else if (lexeme == "short")   type = TOKEN_TYPE::SHORT;
    else if (lexeme == "long")    type = TOKEN_TYPE::LONG;
    else if (lexeme == "char")    type = TOKEN_TYPE::CHAR;
    else if (lexeme == "typedef") type = TOKEN_TYPE::TYPEDEF;
    else if (lexeme == "for")     type = TOKEN_TYPE::FOR;
    else                          type = TOKEN_TYPE::IDENT;

    return Token(type, std::move(lexeme), lineStart, m_line, columnStart, m_column);
}

Token Lexer::parseSymbolicConstant(size_t lineStart, size_t columnStart) {
    char c = getNextChar(), next;

    // Symbolic constant must have a symbol between quotes
    if (c == '\'') {
        return Token(
            TOKEN_TYPE::ERROR,
            error("Symbolic constant can't be empty."),
            lineStart, m_line, columnStart, m_column
        );
    }
    
    if (c == '\\') {
        c = getNextChar();
        next = getNextChar();

        if (next == '\'') {
            switch (c) {
                case 'n':  return Token(TOKEN_TYPE::CONST_SYMB, '\n', lineStart, m_line, columnStart, m_column);
                case 't':  return Token(TOKEN_TYPE::CONST_SYMB, '\t', lineStart, m_line, columnStart, m_column);
                case '\\': return Token(TOKEN_TYPE::CONST_SYMB, '\\', lineStart, m_line, columnStart, m_column);
                case '\'': return Token(TOKEN_TYPE::CONST_SYMB, '\'', lineStart, m_line, columnStart, m_column);
                default:   return Token(TOKEN_TYPE::ERROR, error("Invalid escape sequence."), lineStart, m_line, columnStart, m_column);
            }
        } else {
            return Token(TOKEN_TYPE::ERROR, error("Symbolic constant was never closed."), lineStart, m_line, columnStart, m_column);
        }
    } else {
        next = getNextChar();
    
        if (next == '\'') {
            return Token(TOKEN_TYPE::CONST_SYMB, c, lineStart, m_line, columnStart, m_column);
        } else {
            // Got more that 1 character in symbolic constant
            return Token(
                TOKEN_TYPE::ERROR,
                error("Symbolic constant can't contain more that 1 symbol."),
                lineStart, m_line, columnStart, m_column
            );
        }
    }
}

Token Lexer::parseStringConstant(size_t lineStart, size_t columnStart) {
    std::string lexeme;
    char c = getNextChar();
    
    // Quote is closed immediately - empty string
    if (c == '\"') return Token(TOKEN_TYPE::CONST_STR, "", lineStart, m_line, columnStart, m_column);

    while (c != '\"' && c != '\0') {
        if (c == '\\') {
            char escaped = getNextChar();
            switch (escaped) {
                case 'n':  lexeme += '\n'; m_column += 2; break;
                case 't':  lexeme += '\t'; m_column += 2; break;
                case '\\': lexeme += '\\'; m_column += 2; break;
                case '\"': lexeme += '\"'; m_column += 2; break;
                default:   return Token(TOKEN_TYPE::ERROR, error("Invalid escape sequence."), lineStart, m_line, columnStart, m_column);
            }
        } else {
            lexeme += c;
        }

        c = getNextChar();
    }

    // Quote ("") was never closed
    if (c == '\0') {
        return Token(TOKEN_TYPE::ERROR, error("String constant was never closed"), lineStart, m_line, columnStart, m_column);
    }

    return Token(TOKEN_TYPE::CONST_STR, std::move(lexeme), lineStart, m_line, columnStart, m_column);
}

Token Lexer::parseIdentifier(char firstCharacter, size_t lineStart, size_t columnStart) {
    std::string lexeme;
    lexeme += firstCharacter;

    char next = getNextChar();

    while (std::isalpha(next) || std::isdigit(next) || next == '_') {
        lexeme += next;

        // Limit the identifier length
        if (lexeme.length() > MAX_IDENTIFIER_LENGTH) {
            return Token(
                TOKEN_TYPE::ERROR, 
                std::format("The length of an identifier must not exceed {} characters.", MAX_IDENTIFIER_LENGTH), 
                lineStart, m_line, columnStart, m_column
            );
        }

        next = getNextChar();
    }

    returnCharToBuffer(next);

    // Check if token is a keyword or an identifier
    return lookupKeyword(lexeme, lineStart, columnStart); 
}

Token Lexer::parseNumericConstant(char firstDigit, size_t lineStart, size_t columnStart) {
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
                return Token(
                    TOKEN_TYPE::ERROR,
                    error("Hex constant is too long."),
                    lineStart, m_line, columnStart, m_column
                );
            }

            c = getNextChar();
        } while (std::isxdigit(static_cast<unsigned char>(c)));

        // Invalid: "0x" with no digits after
        if (std::tolower(lexeme.back()) == 'x') {
            return Token(
                TOKEN_TYPE::ERROR,
                error("Invalid hex constant."),
                lineStart, m_line, columnStart, m_column
            );
        }

        returnCharToBuffer(c);

        return Token(
            TOKEN_TYPE::CONST_HEX,
            std::move(lexeme),
            lineStart, m_line, columnStart, m_column
        );
    } else {
        // Decimal constant
        while (std::isdigit(c)) {
            lexeme += c;

            // len(2147483647) = 10, so we can at least discard constants
            // whose length is greater than 10
            if (lexeme.length() > 10) {
                return Token(TOKEN_TYPE::ERROR, error("Decimal constant is too long."), lineStart, m_line, columnStart, m_column);
            }

            c = getNextChar();
        }

        returnCharToBuffer(c);
        return Token(
            TOKEN_TYPE::CONST_DEC,
            std::move(lexeme),
            lineStart, m_line, columnStart, m_column
        );
    }
}

std::string Lexer::error(std::string&& error) const {
    return std::format("{}:{}:{}: {}", m_path, m_line, m_column, error);
}

std::string Lexer::getFilePath() const {
    return m_path;
}

bool Lexer::isLineFeedSkipped() {
    bool t = m_isLineFeedSkipped;
    m_isLineFeedSkipped = false;
    return t;
}