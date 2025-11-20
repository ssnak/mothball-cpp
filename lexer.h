#pragma once

#include <cstdint>
#include <cstring>
#include <string>
enum class TokenType {
    Identifier,
    Modifier,
    Integer,
    Float,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Add,
    Multiply,
    Divide,
    Subtract,
    For,
    While,
    If,
    Boolean,
    Equals,
    LessThan,
    GreaterThan,
    Assign,
    EndOfFile,
    Unknown,
};

struct Token {
    TokenType type;
    std::string lexeme;
    Token(TokenType type, std::string lexeme) : type(type), lexeme(lexeme) {}
    Token() = default;
};

class Lexer {
   private:
    const char* m_yylimit;

   public:
    const char* input;
    uint32_t pos = 0;
    Lexer(const char* input) {
        this->input = input;
        m_yylimit = input + std::strlen(input);
    }

    Token nextToken();
};
