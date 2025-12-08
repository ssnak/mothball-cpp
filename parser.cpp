#include "parser.h"

#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

void BlockStmt::accept(struct StmtVisitor& visitor) { visitor.visitBlockStmt(*this); }
void ExprStmt::accept(struct StmtVisitor& visitor) { visitor.visitExprStmt(*this); }
OptionalValue LiteralExpr::accept(struct ExprVisitor& visitor) { return visitor.visitLiteralExpr(*this); }
OptionalValue BinaryExpr::accept(struct ExprVisitor& visitor) { return visitor.visitBinaryExpr(*this); }
OptionalValue CallExpr::accept(struct ExprVisitor& visitor) { return visitor.visitCallExpr(*this); }

void CodeVisitor::visitExprStmt(ExprStmt& stmt) { stmt.expression->accept(*this); }
void CodeVisitor::visitBlockStmt(BlockStmt& stmt) {
    for (const auto& it : stmt.statements) {
        it.get()->accept(*this);
    }
}

bool stringCheck(std::string& str, std::string sub) {
    if (str.starts_with(sub)) {
        str = str.substr(sub.length());
        return true;
    }
    return false;
}
OptionalValue CodeVisitor::visitLiteralExpr(LiteralExpr& expr) {
    switch (expr.type) {
        case LiteralExpr::Type::String:
            return expr.value;
        case LiteralExpr::Type::Boolean:
            return expr.value == "true";
        case LiteralExpr::Type::Integer:
            return std::stoi(expr.value);
        case LiteralExpr::Type::Float:
            return std::stof(expr.value);
    }
    return std::nullopt;
}

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
template <typename T>
T checkRhs(T rhs) {
    if (rhs == 0) {
        throw std::runtime_error("Can not divide by zero");
    }
    return rhs;
}
OptionalValue CodeVisitor::visitBinaryExpr(BinaryExpr& expr) {
    switch (expr.operation[0]) {
        case '+': {
            return std::visit(
                overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs + rhs; },
                           [](float lhs, float rhs) -> OptionalValue { return lhs + rhs; },
                           [](int lhs, float rhs) -> OptionalValue { return lhs + rhs; },
                           [](float lhs, int rhs) -> OptionalValue { return lhs + rhs; },
                           [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
                expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
        }
        case '-': {
            return std::visit(
                overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs - rhs; },
                           [](float lhs, float rhs) -> OptionalValue { return lhs - rhs; },
                           [](int lhs, float rhs) -> OptionalValue { return lhs - rhs; },
                           [](float lhs, int rhs) -> OptionalValue { return lhs - rhs; },
                           [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
                expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
        }
        case '*': {
            return std::visit(
                overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs * rhs; },
                           [](float lhs, float rhs) -> OptionalValue { return lhs * rhs; },
                           [](int lhs, float rhs) -> OptionalValue { return lhs * rhs; },
                           [](float lhs, int rhs) -> OptionalValue { return lhs * rhs; },
                           [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
                expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
        }
        case '/': {
            return std::visit(
                overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                           [](float lhs, float rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                           [](int lhs, float rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                           [](float lhs, int rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                           [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
                expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
        }
    }
    return std::nullopt;
}
OptionalValue CodeVisitor::visitCallExpr(CallExpr& expr) {
    std::string identifier{expr.identifier};
    if (identifier == "|") {
        m_player.position.x = 0.0f;
        m_player.position.z = 0.0f;
        return std::nullopt;
    } else if (identifier == "facing") {
        if (auto args = std::move(expr.arguments); args.size() > 0) {
            if (auto v = args[0]->accept(*this); v.has_value()) {
                Value value = v.value();
                std::visit(
                    overloaded{[this](int val) { m_player.face(static_cast<float>(val)); },
                               [this](float val) { m_player.face(val); },
                               [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                               [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                    value);
            } else {
                std::cerr << "Error argument not recognized" << std::endl;
            }
        } else {
            m_player.face(0.0f);
        }
        return std::nullopt;
    } else if (identifier == "outx") {
        if (auto args = std::move(expr.arguments); args.size() > 0) {
            if (auto v = args[0]->accept(*this); v.has_value()) {
                Value value = v.value();
                std::visit(
                    overloaded{[this](auto offset) {
                                   if (offset >= m_player.position.x) {
                                       std::cout << "X: " << offset << " - " << std::setprecision(m_player.precision)
                                                 << offset - m_player.position.x << std::endl;
                                   } else {
                                       std::cout << "X: " << offset << " + " << std::setprecision(m_player.precision)
                                                 << m_player.position.x - offset << std::endl;
                                   }
                               },
                               [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                               [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                    value);
            } else {
                std::cerr << "Error argument not recognized" << std::endl;
            }
        } else {
            std::cout << "X: " << std::setprecision(m_player.precision) << m_player.position.x << std::endl;
        }
        return std::nullopt;
    } else if (identifier == "outz") {
        if (auto args = std::move(expr.arguments); args.size() > 0) {
            if (auto v = args[0]->accept(*this); v.has_value()) {
                Value value = v.value();
                std::visit(
                    overloaded{[this](auto offset) {
                                   if (offset >= m_player.position.z) {
                                       std::cout << "Z: " << offset << " - " << std::setprecision(m_player.precision)
                                                 << offset - m_player.position.z << std::endl;
                                   } else {
                                       std::cout << "Z: " << offset << " + " << std::setprecision(m_player.precision)
                                                 << m_player.position.z - offset << std::endl;
                                   }
                               },
                               [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                               [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                    value);
            } else {
                std::cerr << "Error argument not recognized" << std::endl;
            }
        } else {
            std::cout << "Z: " << std::setprecision(m_player.precision) << m_player.position.z << std::endl;
        }
        return std::nullopt;
    } else if (identifier == "outvx") {
        if (auto args = std::move(expr.arguments); args.size() > 0) {
            if (auto v = args[0]->accept(*this); v.has_value()) {
                Value value = v.value();
                std::visit(
                    overloaded{[this](auto offset) {
                                   if (offset >= m_player.velocity.x) {
                                       std::cout << "Vx: " << offset << " - " << std::setprecision(m_player.precision)
                                                 << offset - m_player.velocity.x << std::endl;
                                   } else {
                                       std::cout << "Vx: " << offset << " + " << std::setprecision(m_player.precision)
                                                 << m_player.velocity.x - offset << std::endl;
                                   }
                               },
                               [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                               [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                    value);
            } else {
                std::cerr << "Error argument not recognized" << std::endl;
            }
        } else {
            std::cout << "Vx: " << std::setprecision(m_player.precision) << m_player.velocity.x << std::endl;
        }
        return std::nullopt;
    } else if (identifier == "outvz") {
        if (auto args = std::move(expr.arguments); args.size() > 0) {
            if (auto v = args[0]->accept(*this); v.has_value()) {
                Value value = v.value();
                std::visit(
                    overloaded{[this](auto offset) {
                                   if (offset >= m_player.velocity.z) {
                                       std::cout << "Vz: " << offset << " - " << std::setprecision(m_player.precision)
                                                 << offset - m_player.velocity.z << std::endl;
                                   } else {
                                       std::cout << "Vz: " << offset << " + " << std::setprecision(m_player.precision)
                                                 << m_player.velocity.z - offset << std::endl;
                                   }
                               },
                               [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                               [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                    value);
            } else {
                std::cerr << "Error argument not recognized" << std::endl;
            }
        } else {
            std::cout << "Vz: " << std::setprecision(m_player.precision) << m_player.velocity.z << std::endl;
        }
        return std::nullopt;
    }

    bool isSneaking = false;
    bool isSprinting = false;
    State state = State::GROUNDED;
    std::optional<float> slipperiness = std::nullopt;
    m_player.inputs = expr.inputs;
    if (m_player.inputs.empty()) m_player.inputs = "w";
    // std::vector<std::vector<std::string>> keywords{{"sneak"}, {"walk", "sprint", "stop"}, {"jump", "air", "ground"}};
    if (stringCheck(identifier, "sneak")) {
        isSneaking = true;
    }
    if (stringCheck(identifier, "sprint")) {
        isSprinting = true;
    } else if (stringCheck(identifier, "stop")) {
        m_player.inputs = "";
    } else {
        stringCheck(identifier, "walk");
    }
    if (stringCheck(identifier, "jump")) {
        state = State::JUMPING;
    } else if (stringCheck(identifier, "air")) {
        state = State::AIRBORNE;
        slipperiness = 1.0f;
    }
    // std::cout << "Args: " << expr.arguments.size() << std::endl;
    int duration = 1;
    if (auto args = std::move(expr.arguments); args.size() > 0) {
        if (auto v = args[0]->accept(*this); v.has_value()) {
            Value value = v.value();
            std::visit(overloaded{[&duration](int val) { duration = val; },
                                  [&duration](float val) { duration = static_cast<int>(val); },
                                  [](bool) { std::cerr << "Expected int got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected int got string instead" << std::endl; }},
                       value);
        } else {
            std::cerr << "Error argument not recognized" << std::endl;
        }
    }
    m_player.move(duration, std::nullopt, 0.0f, slipperiness, isSprinting, isSneaking, std::nullopt, std::nullopt,
                  state);
    std::cout << m_player;
    return std::nullopt;
}
