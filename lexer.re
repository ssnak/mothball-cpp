#include "lexer.h"
#define s_token std::string(start, m_cursor)
#define c_token std::string(m_cursor - 1, 1)

Token Lexer::next() {
    const char* YYMARKER;
    const char* start;
    /*!stags:re2c format = 'const char *@@;\n'; */

loop:
    /*!re2c
        re2c:eof = 0;
        re2c:api:style = free-form;
        re2c:yyfill:enable = 0;
        re2c:define:YYCTYPE = char;
        re2c:define:YYCURSOR = m_cursor;
        re2c:define:YYLIMIT = m_limit;
        re2c:tags = 1;
        modifier = ("["[a-zA-Z]+"]")|("."[wasd]{1,4});
        number = "0"|[1-9][0-9]*;
        boolean = "true"|"false";
        string = "'"[^']*"'";
        identifier = [a-zA-Z_]([a-zA-Z_]|number)*;
        builtin =
       ("|"|"f"("acing")?|"outx"|"outz"|"xmm"|"zmm"|"xb"|"zb"|"outvx"|"outvz"|"setx"|"setz"|"setvx"|"setvz"|"print");
        movement = ("sn"("eak")?)?("s"("print")?|"st"("op")?|"w"("alk")?)?("j"("ump")?|"a"("ir")?)?"45"?;

        @start string          { return Token(TokenType::String, s_token); }
        @start "let"           { return Token(TokenType::Let, s_token); }
        @start "fn"            { return Token(TokenType::FuncDecl, s_token); }
        @start "for"           { return Token(TokenType::For, s_token); }
        @start "while"         { return Token(TokenType::While, s_token); }
        @start "if"            { return Token(TokenType::If, s_token); }
        @start "else"          { return Token(TokenType::Else, s_token); }
        @start boolean         { return Token(TokenType::Boolean, s_token); }
        @start "=="            { return Token(TokenType::Equals, s_token); }
        @start "!="            { return Token(TokenType::NotEquals, s_token); }
        @start ">="            { return Token(TokenType::GreaterThanOrEquals, s_token); }
        @start "<="            { return Token(TokenType::LessThanOrEquals, s_token); }
        @start "&&"            { return Token(TokenType::And, s_token); }
        @start "||"            { return Token(TokenType::Or, s_token); }
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
        ";"                    { return Token(TokenType::Semicolon, c_token); }
        @start builtin         { return Token(TokenType::Builtin, s_token); }
        @start movement        { return Token(TokenType::Movement, s_token); }
        @start identifier      { return Token(TokenType::Identifier, s_token); }
        @start modifier        { return Token(TokenType::Modifier, s_token); }
        @start number          { return Token(TokenType::Integer, s_token); }
        @start number"."number { return Token(TokenType::Float, s_token); }
        [ \t\n\r"\\"]+         { goto loop; }
        *                      { return Token(TokenType::Unknown, "UNKNOWN"); }
        $                      { return Token(TokenType::EndOfFile, "EOF"); }
    */
}
