#include "token.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <cstdint>
#include <optional>


class Lexer {
public:
    Lexer(const std::string& path);
    ~Lexer();

public:
    // Main method that parses a file into a sequence of tokens
    std::vector<Token> parse();

private:
    // Refill the character buffer from the input stream
    bool refillBuffer();
    // Determine the next token
    Token getNextToken();
    // Read the next character from a buffer
    char getNextChar();
    // Return taken character back to a buffer
    void returnCharToBuffer(char c);
    // Goes through buffer until there's no whitespaces and comments
    char skipWhitespacesAndComments();
    // Methods used to parse constants and keywords/identifiers
    Token parseSymbolicConstant();
    Token parseStringConstant();
    Token parseIdentifier(char firstCharacter);
    Token parseNumericConstant(char firstDigit);
    Token lookupKeyword(std::string& lexeme) const;

private:
    static const int BUFFER_SIZE = 16384; // Main character buffer max size
    const int MAX_IDENTIFIER_LENGTH = 32; // Maximum length of a single identifier

    std::ifstream m_inputStream;            // Input stream object
    std::array<char, BUFFER_SIZE> m_buffer; // Character buffer
    std::optional<char> m_pushback;         // Variable used to store the character which can't be directly returned to buffer

    std::size_t m_validSize; // Actual number of  characters read from the input stream
    std::size_t m_currIndex; // Read pointer
};