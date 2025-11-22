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
    std::string text;
    Token(TokenType type, std::string text) : type(type), text(text) {}
    Token() = default;
};

class Lexer {
   private:
    const char* m_yylimit;
    const char* m_input;

   public:
    Lexer(const char* input) {
        m_input = input;
        m_yylimit = m_input + std::strlen(m_input);
    }

    Token next();
};
