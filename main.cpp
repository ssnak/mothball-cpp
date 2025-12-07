#include <cstdio>

#include "lexer.h"
#include "parser.h"
#include "player.h"

int main() {
    std::string input = "sprintjump 2 + 5.0 * 2 walk.s sprintjump sprintair";
    Scanner scanner(input);
    CodeVisitor vis;
    scanner.scan().accept(vis);
    // std::string input =
    //     "2315 + 531.21\n while x < 5 { x = 5 }\n -sj.wa 12\n if jumping == true {jumping = false} dal234h;6ajsdf90  "
    //     "50 "
    //     "* 2 - 3 + 5 / 8";
    // Lexer lex(input);
    // Token t;
    // while (t.type != TokenType::EndOfFile) {
    //     t = lex.next();
    //     printf("%d %s\n", static_cast<int>(t.type), t.text.c_str());
    // }
    //
    // Player player;
    // player.inputs = "w";
    // player.sprintjump(12);
    // printf("%f %f\n", player.velocity.z, player.position.z);
}
