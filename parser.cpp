#include "parser.h"

#include <iostream>
#include <optional>

#include "player.h"
Player player;
void BlockStmt::accept(struct StmtVisitor& visitor) { visitor.visitBlockStmt(*this); }
void ExprStmt::accept(struct StmtVisitor& visitor) { visitor.visitExprStmt(*this); }
void CallExpr::accept(struct ExprVisitor& visitor) { visitor.visitCallExpr(*this); }
bool check(std::string& str, std::string sub) {
    if (str.starts_with(sub)) {
        str = str.substr(sub.length());
        return true;
    }
    return false;
}
void CodeVisitor::visitExprStmt(ExprStmt& stmt) { stmt.expression->accept(*this); }
void CodeVisitor::visitCallExpr(CallExpr& expr) {
    std::string identifier{expr.identifier};
    bool isSneaking = false;
    bool isSprinting = false;
    State state = State::GROUNDED;
    std::optional<float> slipperiness = std::nullopt;
    player.inputs = expr.inputs;
    if (player.inputs.empty()) player.inputs = "w";
    if (check(identifier, "sneak")) {
        isSneaking = true;
    }
    if (check(identifier, "sprint")) {
        isSprinting = true;
    } else if (check(identifier, "stop")) {
        player.inputs = "";
    } else {
        check(identifier, "walk");
    }
    if (check(identifier, "jump")) {
        state = State::JUMPING;
    } else if (check(identifier, "air")) {
        state = State::AIRBORNE;
        slipperiness = 1.0f;
    }
    player.move(12, std::nullopt, 0.0f, slipperiness, isSprinting, isSneaking, std::nullopt, std::nullopt, state);
    std::cout << player;
}
void CodeVisitor::visitBlockStmt(BlockStmt& stmt) {
    for (const auto& it : stmt.statements) {
        it.get()->accept(*this);
    }
}
