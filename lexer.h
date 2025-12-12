#pragma once

#include <string>
enum class TokenType {
    Identifier,
    Builtin,
    Movement,
    Modifier,
    Integer,
    Float,
    String,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Add,
    Multiply,
    Divide,
    Subtract,
    Let,
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
    std::string m_input;
    const char* m_cursor;
    const char* m_limit;

   public:
    Lexer(const std::string& input) : m_input(input) {
        m_cursor = m_input.c_str();
        m_limit = m_cursor + m_input.length();
    }

    Token next();
};
