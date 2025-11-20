#include "lexer.h"
#define s_token std::string(start, input)
#define c_token std::string(input - 1, 1)

Token Lexer::nextToken() {
    const char* YYLIMIT = input + std::strlen(input);
    const char* YYMARKER;
    const char* start;
    /*!stags:re2c format = 'const char *@@;\n'; */

loop:
    /*!re2c
        re2c:eof = 0;
        re2c:api:style = free-form;
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = char;
        re2c:define:YYCURSOR = input;
        re2c:tags = 1;
        identifier = "|"|([a-zA-Z]+"45"?);
        modifier = ("["[a-zA-Z]+"]")|("."[wasd]{1,4});
        number = [0-9]+;
        boolean = "true"|"false";

        @start "for"           { return Token(TokenType::For, s_token); }
        @start "while"         { return Token(TokenType::While, s_token); }
        @start "if"            { return Token(TokenType::If, s_token); }
        @start boolean         { return Token(TokenType::Boolean, s_token); }
        @start "=="            { return Token(TokenType::Equals, s_token); }
        "="                    { return Token(TokenType::Assign, c_token); }
        ">"                    { return Token(TokenType::GreaterThan, c_token); }
        "<"                    { return Token(TokenType::LessThan, c_token); }
        "("                    { return Token(TokenType::LeftParen, c_token); }
        ")"                    { return Token(TokenType::RightParen, c_token); }
        "{"                    { return Token(TokenType::LeftBrace, c_token); }
        "}"                    { return Token(TokenType::RightBrace, c_token); }
        "+"                    { return Token(TokenType::Add, c_token); }
        "-"                    { return Token(TokenType::Subtract, c_token); }
        "*"                    { return Token(TokenType::Multiply, c_token); }
        "/"                    { return Token(TokenType::Divide, c_token); }
        @start identifier      { return Token(TokenType::Identifier, s_token); }
        @start modifier        { return Token(TokenType::Modifier, s_token); }
        @start number          { return Token(TokenType::Integer, s_token); }
        @start number"."number { return Token(TokenType::Float, s_token); }
        [ \t\n\r]+             { goto loop; }
        *                      { return Token(TokenType::Unknown, "UNKNOWN"); }
        $                      { return Token(TokenType::EndOfFile, "EOF"); }
    */
}
