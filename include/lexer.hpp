#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <cstdint>
#include <optional>


class Lexer {
public:
    Lexer(std::string_view path);
    ~Lexer();

public:
    // Determine the next token
    Token getNextToken();
    std::string getFilePath() const;
    bool isLineFeedSkipped();

private:
    // Input stream handlers
    void openStream();
    void closeStream();
    // Refill the character buffer from the input stream
    bool refillBuffer();
    
    // Read the next character from a buffer
    char getNextChar();
    // Return taken character back to a buffer
    void returnCharToBuffer(char c);
    // Goes through buffer until there's no whitespaces and comments
    char skipWhitespacesAndComments();
    // Methods used to parse constants and keywords/identifiers
    Token parseSymbolicConstant(size_t lineStart, size_t columnStart);
    Token parseStringConstant(size_t lineStart, size_t columnStart);
    Token parseIdentifier(char firstCharacter, size_t lineStart, size_t columnStart);
    Token parseNumericConstant(char firstDigit, size_t lineStart, size_t columnStart);
    Token lookupKeyword(std::string& lexeme, size_t lineStart, size_t lineColumn);
    // Formats the error string so that it containts the error line and column
    std::string error(std::string&& error) const;

private:
    std::string m_path; // Path to the currently parsed file
    static constexpr size_t BUFFER_SIZE = 16384; // Main character buffer max size
    static constexpr size_t MAX_IDENTIFIER_LENGTH = 32; // Maximum length of a single identifier

    std::ifstream m_inputStream;            // Input stream object
    std::array<char, BUFFER_SIZE> m_buffer; // Character buffer
    std::optional<char> m_pushback;         // Variable used to store the character which can't be directly returned to buffer

    std::size_t m_validSize;  // Actual number of characters read from the input stream
    std::size_t m_currIndex;  // Read pointer
    std::size_t m_line;       // Line number in the currently parsed file
    std::size_t m_column;     // Column number in the currently parsed file
    std::size_t m_prevColumn; // Column number of the previous character from the stream
    bool m_isLineFeedSkipped; // Flag indicating if '\n' symbol was read
};

#endif // LEXER_HPP