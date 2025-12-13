#include "parser.h"

#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

void BlockStmt::accept(struct StmtVisitor& visitor) { visitor.visitBlockStmt(*this); }
void ExprStmt::accept(struct StmtVisitor& visitor) { visitor.visitExprStmt(*this); }
void ForStmt::accept(struct StmtVisitor& visitor) { visitor.visitForStmt(*this); }
void VarDeclStmt::accept(struct StmtVisitor& visitor) { visitor.visitVarDeclStmt(*this); }
OptionalValue LiteralExpr::accept(struct ExprVisitor& visitor) { return visitor.visitLiteralExpr(*this); }
OptionalValue VarExpr::accept(struct ExprVisitor& visitor) { return visitor.visitVarExpr(*this); }
OptionalValue UnaryExpr::accept(struct ExprVisitor& visitor) { return visitor.visitUnaryExpr(*this); }
OptionalValue BinaryExpr::accept(struct ExprVisitor& visitor) { return visitor.visitBinaryExpr(*this); }
OptionalValue CallExpr::accept(struct ExprVisitor& visitor) { return visitor.visitCallExpr(*this); }

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
void CodeVisitor::visitExprStmt(ExprStmt& stmt) { stmt.expression->accept(*this); }
void CodeVisitor::visitBlockStmt(BlockStmt& stmt) {
    size_t variablesSize = m_variables.size();
    for (const auto& it : stmt.statements) {
        it.get()->accept(*this);
    }
    m_variables.resize(variablesSize);
}
void CodeVisitor::visitForStmt(ForStmt& stmt) {
    int times = 0;
    std::visit(
        overloaded{[&times](int value) { times = value; }, [&times](float value) { times = static_cast<int>(value); },
                   [](auto) { throw std::runtime_error("Invalid expression for loop"); }},
        stmt.condition->accept(*this).value());
    for (int i = 0; i < times; i++) {
        stmt.body->accept(*this);
    }
}
void CodeVisitor::visitVarDeclStmt(VarDeclStmt& stmt) {
    m_variables.push_back(Var{stmt.identifier, stmt.value->accept(*this)});
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

OptionalValue CodeVisitor::visitVarExpr(VarExpr& expr) {
    for (int i = m_variables.size() - 1; i >= 0; i--) {
        if (m_variables[i].identifier == expr.identifier) {
            return m_variables[i].value;
        }
    }
    std::cerr << "Variable not recognized" << std::endl;
    return std::nullopt;
}

template <typename T>
T checkRhs(T rhs) {
    if (rhs == 0) {
        throw std::runtime_error("Can not divide by zero");
    }
    return rhs;
}
OptionalValue CodeVisitor::visitUnaryExpr(UnaryExpr& expr) {
    switch (expr.operation[0]) {
        case '-': {
            return std::visit(
                overloaded{[](int lhs) -> OptionalValue { return -lhs; },
                           [](float lhs) -> OptionalValue { return -lhs; },
                           [](auto) -> OptionalValue { throw std::runtime_error("Invalid operands for unary minus"); }},
                expr.operand->accept(*this).value());
        }
        case '+': {
            return std::visit(
                overloaded{[](int lhs) -> OptionalValue { return lhs; }, [](float lhs) -> OptionalValue { return lhs; },
                           [](auto) -> OptionalValue { throw std::runtime_error("Invalid operands for unary plus"); }},
                expr.operand->accept(*this).value());
        }
    }
    return std::nullopt;
}

OptionalValue CodeVisitor::visitBinaryExpr(BinaryExpr& expr) {
    if (expr.operation == "+") {
        return std::visit(
            overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs + rhs; },
                       [](float lhs, float rhs) -> OptionalValue { return lhs + rhs; },
                       [](int lhs, float rhs) -> OptionalValue { return lhs + rhs; },
                       [](float lhs, int rhs) -> OptionalValue { return lhs + rhs; },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "-") {
        return std::visit(
            overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs - rhs; },
                       [](float lhs, float rhs) -> OptionalValue { return lhs - rhs; },
                       [](int lhs, float rhs) -> OptionalValue { return lhs - rhs; },
                       [](float lhs, int rhs) -> OptionalValue { return lhs - rhs; },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "*") {
        return std::visit(
            overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs * rhs; },
                       [](float lhs, float rhs) -> OptionalValue { return lhs * rhs; },
                       [](int lhs, float rhs) -> OptionalValue { return lhs * rhs; },
                       [](float lhs, int rhs) -> OptionalValue { return lhs * rhs; },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "/") {
        return std::visit(
            overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                       [](float lhs, float rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                       [](int lhs, float rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                       [](float lhs, int rhs) -> OptionalValue { return lhs / checkRhs(rhs); },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for add"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "<") {
        return std::visit(
            overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs < rhs; },
                       [](float lhs, float rhs) -> OptionalValue { return lhs < rhs; },
                       [](int lhs, float rhs) -> OptionalValue { return lhs < rhs; },
                       [](float lhs, int rhs) -> OptionalValue { return lhs < rhs; },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for less than"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == ">") {
        return std::visit(overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs > rhs; },
                                     [](float lhs, float rhs) -> OptionalValue { return lhs > rhs; },
                                     [](int lhs, float rhs) -> OptionalValue { return lhs > rhs; },
                                     [](float lhs, int rhs) -> OptionalValue { return lhs > rhs; },
                                     [](auto, auto) -> OptionalValue {
                                         throw std::runtime_error("Invalid operands for greater than");
                                     }},
                          expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "==") {
        return std::visit(
            overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs == rhs; },
                       [](float lhs, float rhs) -> OptionalValue { return lhs == rhs; },
                       [](int lhs, float rhs) -> OptionalValue { return lhs == rhs; },
                       [](float lhs, int rhs) -> OptionalValue { return lhs == rhs; },
                       [](bool lhs, bool rhs) -> OptionalValue { return lhs == rhs; },
                       [](std::string lhs, std::string rhs) -> OptionalValue { return lhs == rhs; },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for equals"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "!=") {
        return std::visit(overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs != rhs; },
                                     [](float lhs, float rhs) -> OptionalValue { return lhs != rhs; },
                                     [](int lhs, float rhs) -> OptionalValue { return lhs != rhs; },
                                     [](float lhs, int rhs) -> OptionalValue { return lhs != rhs; },
                                     [](bool lhs, bool rhs) -> OptionalValue { return lhs != rhs; },
                                     [](std::string lhs, std::string rhs) -> OptionalValue { return lhs != rhs; },
                                     [](auto, auto) -> OptionalValue {
                                         throw std::runtime_error("Invalid operands for not equals");
                                     }},
                          expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == ">=") {
        return std::visit(overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs >= rhs; },
                                     [](float lhs, float rhs) -> OptionalValue { return lhs >= rhs; },
                                     [](int lhs, float rhs) -> OptionalValue { return lhs >= rhs; },
                                     [](float lhs, int rhs) -> OptionalValue { return lhs >= rhs; },
                                     [](auto, auto) -> OptionalValue {
                                         throw std::runtime_error("Invalid operands for greater than or equals");
                                     }},
                          expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "<=") {
        return std::visit(overloaded{[](int lhs, int rhs) -> OptionalValue { return lhs <= rhs; },
                                     [](float lhs, float rhs) -> OptionalValue { return lhs <= rhs; },
                                     [](int lhs, float rhs) -> OptionalValue { return lhs <= rhs; },
                                     [](float lhs, int rhs) -> OptionalValue { return lhs <= rhs; },
                                     [](auto, auto) -> OptionalValue {
                                         throw std::runtime_error("Invalid operands for less than or equals");
                                     }},
                          expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "&&") {
        // TODO: Short circuit if first operand is false
        return std::visit(
            overloaded{[](bool lhs, bool rhs) -> OptionalValue { return lhs && rhs; },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for and"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    } else if (expr.operation == "||") {
        // TODO: Short circuit if first operand is true
        return std::visit(
            overloaded{[](bool lhs, bool rhs) -> OptionalValue { return lhs || rhs; },
                       [](auto, auto) -> OptionalValue { throw std::runtime_error("Invalid operands for or"); }},
            expr.lhs->accept(*this).value(), expr.rhs->accept(*this).value());
    }

    return std::nullopt;
}

OptionalValue CodeVisitor::visitCallExpr(CallExpr& expr) {
    std::string identifier{expr.identifier};
    if (identifier == "|") {
        m_player.position.x = 0.0f;
        m_player.position.z = 0.0f;
        return std::nullopt;
    }

    std::vector<Value> args;
    for (auto& arg : expr.arguments) {
        if (auto result = arg->accept(*this); result.has_value()) {
            args.push_back(result.value());
        } else {
            std::cerr << "Error invalid argument" << std::endl;
        }
    }

    if (identifier == "facing" || identifier == "f") {
        if (args.size() > 0) {
            std::visit(overloaded{[this](int val) { m_player.face(static_cast<float>(val)); },
                                  [this](float val) { m_player.face(val); },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            m_player.face(0.0f);
        }
        return std::nullopt;
    }
    if (identifier == "outx") {
        if (args.size() > 0) {
            std::visit(overloaded{[this](auto offset) {
                                      if (offset >= m_player.position.x) {
                                          std::cout << "x: " << offset << " - " << std::setprecision(m_player.precision)
                                                    << offset - m_player.position.x << std::endl;
                                      } else {
                                          std::cout << "x: " << offset << " + " << std::setprecision(m_player.precision)
                                                    << m_player.position.x - offset << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "X: " << std::setprecision(m_player.precision) << m_player.position.x << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "outz") {
        if (args.size() > 0) {
            std::visit(overloaded{[this](auto offset) {
                                      if (offset >= m_player.position.z) {
                                          std::cout << "z: " << offset << " - " << std::setprecision(m_player.precision)
                                                    << offset - m_player.position.z << std::endl;
                                      } else {
                                          std::cout << "z: " << offset << " + " << std::setprecision(m_player.precision)
                                                    << m_player.position.z - offset << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "z: " << std::setprecision(m_player.precision) << m_player.position.z << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "xmm") {
        float pos = m_player.position.x;
        if (pos >= 0.6f) {
            pos -= 0.6f;
        } else if (pos <= -0.6f) {
            pos += 0.6f;
        } else {
            pos = 0.0f;
        }
        if (args.size() > 0) {
            std::visit(overloaded{[this, &pos](auto offset) {
                                      if (offset >= pos) {
                                          std::cout << "x(mm): " << offset << " - "
                                                    << std::setprecision(m_player.precision) << offset - pos
                                                    << std::endl;
                                      } else {
                                          std::cout << "x(mm): " << offset << " + "
                                                    << std::setprecision(m_player.precision) << pos - offset
                                                    << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "x(mm): " << std::setprecision(m_player.precision) << pos << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "zmm") {
        float pos = m_player.position.z;
        if (pos >= 0.6f) {
            pos -= 0.6f;
        } else if (pos <= -0.6f) {
            pos += 0.6f;
        } else {
            pos = 0.0f;
        }
        if (args.size() > 0) {
            std::visit(overloaded{[this, &pos](auto offset) {
                                      if (offset >= pos) {
                                          std::cout << "z(mm): " << offset << " - "
                                                    << std::setprecision(m_player.precision) << offset - pos
                                                    << std::endl;
                                      } else {
                                          std::cout << "z(mm): " << offset << " + "
                                                    << std::setprecision(m_player.precision) << pos - offset
                                                    << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "z(mm): " << std::setprecision(m_player.precision) << pos << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "xb") {
        float pos = m_player.position.x;
        if (pos >= 0.0f) {
            pos += 0.6f;
        } else {
            pos -= 0.6f;
        }
        if (args.size() > 0) {
            std::visit(overloaded{[this, &pos](auto offset) {
                                      if (offset >= pos) {
                                          std::cout << "x(b): " << offset << " - "
                                                    << std::setprecision(m_player.precision) << offset - pos
                                                    << std::endl;
                                      } else {
                                          std::cout << "x(b): " << offset << " + "
                                                    << std::setprecision(m_player.precision) << pos - offset
                                                    << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "x(b): " << std::setprecision(m_player.precision) << pos << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "zb") {
        float pos = m_player.position.z;
        if (pos >= 0.0f) {
            pos += 0.6f;
        } else {
            pos -= 0.6f;
        }
        if (args.size() > 0) {
            std::visit(overloaded{[this, &pos](auto offset) {
                                      if (offset >= pos) {
                                          std::cout << "z(b): " << offset << " - "
                                                    << std::setprecision(m_player.precision) << offset - pos
                                                    << std::endl;
                                      } else {
                                          std::cout << "z(b): " << offset << " + "
                                                    << std::setprecision(m_player.precision) << pos - offset
                                                    << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "z(b): " << std::setprecision(m_player.precision) << pos << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "outvx") {
        if (args.size() > 0) {
            std::visit(overloaded{[this](auto offset) {
                                      if (offset >= m_player.velocity.x) {
                                          std::cout << "Vx: " << offset << " - "
                                                    << std::setprecision(m_player.precision)
                                                    << offset - m_player.velocity.x << std::endl;
                                      } else {
                                          std::cout << "Vx: " << offset << " + "
                                                    << std::setprecision(m_player.precision)
                                                    << m_player.velocity.x - offset << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "Vx: " << std::setprecision(m_player.precision) << m_player.velocity.x << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "outvz") {
        if (args.size() > 0) {
            std::visit(overloaded{[this](auto offset) {
                                      if (offset >= m_player.velocity.z) {
                                          std::cout << "Vz: " << offset << " - "
                                                    << std::setprecision(m_player.precision)
                                                    << offset - m_player.velocity.z << std::endl;
                                      } else {
                                          std::cout << "Vz: " << offset << " + "
                                                    << std::setprecision(m_player.precision)
                                                    << m_player.velocity.z - offset << std::endl;
                                      }
                                  },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            std::cout << "Vz: " << std::setprecision(m_player.precision) << m_player.velocity.z << std::endl;
        }
        return std::nullopt;
    }
    if (identifier == "setx") {
        if (args.size() > 0) {
            std::visit(overloaded{[this](int offset) { m_player.position.x = static_cast<float>(offset); },
                                  [this](float offset) { m_player.position.x = offset; },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            m_player.position.x = 0.0f;
        }
        return std::nullopt;
    }
    if (identifier == "setz") {
        if (args.size() > 0) {
            std::visit(overloaded{[this](int offset) { m_player.position.z = static_cast<float>(offset); },
                                  [this](float offset) { m_player.position.z = offset; },
                                  [](bool) { std::cerr << "Expected float got bool instead" << std::endl; },
                                  [](std::string) { std::cerr << "Expected float got string instead" << std::endl; }},
                       args[0]);
        } else {
            m_player.position.z = 0.0f;
        }
        return std::nullopt;
    }
    if (identifier == "print") {
        if (args.size() > 0) {
            for (auto& arg : args) {
                std::visit(
                    overloaded{[](auto arg) { std::cout << arg; }, [](bool arg) { std::cout << std::boolalpha << arg; },
                               [](std::string& str) { std::cout << str.substr(1, str.size() - 2); }},
                    arg);
            }
            std::cout << std::endl;
        } else {
            std::cerr << "Nothing to print" << std::endl;
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
    if (stringCheck(identifier, "sneak") || stringCheck(identifier, "sn")) {
        isSneaking = true;
    }
    if (stringCheck(identifier, "stop") || stringCheck(identifier, "st")) {
        m_player.inputs = "";
    } else if (stringCheck(identifier, "sprint") || stringCheck(identifier, "s")) {
        isSprinting = true;
    } else {
        stringCheck(identifier, "walk") || stringCheck(identifier, "w");
    }
    if (stringCheck(identifier, "jump") || stringCheck(identifier, "j")) {
        state = State::JUMPING;
    } else if (stringCheck(identifier, "air") || stringCheck(identifier, "a")) {
        state = State::AIRBORNE;
        slipperiness = 1.0f;
    }
    float offset = 0.0f;
    if (stringCheck(identifier, "45")) offset = 45.0f;
    int duration = 1;
    std::optional<float> rotation = std::nullopt;
    if (args.size() > 0) {
        std::visit(overloaded{[&duration](int value) { duration = value; },
                              [&duration](float value) { duration = static_cast<int>(value); },
                              [](bool) { std::cerr << "Expected int got bool instead" << std::endl; },
                              [](std::string) { std::cerr << "Expected int got string instead" << std::endl; }},
                   args[0]);
    }
    if (args.size() > 1) {
        std::visit(overloaded{[&rotation](int value) { rotation = static_cast<float>(value); },
                              [&rotation](float value) { rotation = value; },
                              [](bool) { std::cerr << "Expected int got bool instead" << std::endl; },
                              [](std::string) { std::cerr << "Expected int got string instead" << std::endl; }},
                   args[1]);
    }
    if (offset == 45.0f && state == State::JUMPING) {
        m_player.move(1, rotation, 0.0f, slipperiness, isSprinting, isSneaking, std::nullopt, std::nullopt, state);
        m_player.move(duration - 1, rotation, offset, 1.0f, isSprinting, isSneaking, std::nullopt, std::nullopt,
                      State::AIRBORNE);
    } else {
        m_player.move(duration, rotation, offset, slipperiness, isSprinting, isSneaking, std::nullopt, std::nullopt,
                      state);
    }
    return std::nullopt;
}
