#include "parser.h"

int main() {
    std::string input =
        "facing 30.3 stopjump walkair.s 0 sneak.s 4 stop stopjump outvz sprintair outz outvz sprintair 10 sprint 1 "
        "outz 1 outx -0.4 | sprintjump 9 outz -0.6 outx 0.6";
    Scanner scanner(input);
    CodeVisitor visitor;
    scanner.scan().accept(visitor);
}
