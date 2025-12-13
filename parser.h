#pragma once
#include <regex.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "lexer.h"
#include "player.h"

using Value = std::variant<int, float, bool, std::string>;
using OptionalValue = std::optional<Value>;
struct Expr {
    virtual ~Expr() = default;
    virtual OptionalValue accept(struct ExprVisitor& visitor) = 0;
};

struct LiteralExpr : public Expr {
    enum class Type { Integer, Float, Boolean, String };
    Type type;
    std::string value;
    OptionalValue accept(struct ExprVisitor& visitor) override;
    LiteralExpr(Type type, std::string value) : type(type), value(value) {}
};

struct VarExpr : public Expr {
    std::string identifier;
    OptionalValue accept(struct ExprVisitor& visitor) override;
    VarExpr(std::string identifier) : identifier(identifier) {}
};

struct CallExpr : public Expr {
   public:
    std::string identifier;
    std::vector<std::string> modifiers;
    std::string inputs;
    std::vector<std::unique_ptr<Expr>> arguments;
    OptionalValue accept(struct ExprVisitor& visitor) override;
};

struct UnaryExpr : public Expr {
    std::unique_ptr<Expr> operand;
    std::string operation;
    OptionalValue accept(struct ExprVisitor& visitor) override;
    UnaryExpr() {}
    explicit UnaryExpr(std::unique_ptr<Expr> operand, std::string& operation)
        : operand(std::move(operand)), operation{operation} {}
};

struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> lhs;
    std::string operation;
    std::unique_ptr<Expr> rhs;
    OptionalValue accept(struct ExprVisitor& visitor) override;

    BinaryExpr() {}
    explicit BinaryExpr(std::unique_ptr<Expr> lhs, std::string& op, std::unique_ptr<Expr> rhs)
        : lhs(std::move(lhs)), operation{op}, rhs(std::move(rhs)) {}
};

struct ExprVisitor {
    virtual OptionalValue visitLiteralExpr(LiteralExpr& expr) = 0;
    virtual OptionalValue visitVarExpr(VarExpr& expr) = 0;
    virtual OptionalValue visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual OptionalValue visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual OptionalValue visitCallExpr(CallExpr& expr) = 0;
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(struct StmtVisitor& visitor) = 0;
};

struct ExprStmt : public Stmt {
    std::unique_ptr<Expr> expression;
    void accept(struct StmtVisitor& visitor) override;
    ExprStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
};

struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    void accept(struct StmtVisitor& visitor) override;
};

struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    void accept(struct StmtVisitor& visitor) override;
};

struct ForStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    void accept(struct StmtVisitor& visitor) override;
};

struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    void accept(struct StmtVisitor& visitor) override;
};

struct VarDeclStmt : public Stmt {
    std::string identifier;
    std::unique_ptr<Expr> value;
    void accept(struct StmtVisitor& visitor) override;
};

struct FuncDeclStmt : public Stmt {
    void accept(struct StmtVisitor& visitor) override;
};

struct StmtVisitor {
    virtual void visitExprStmt(ExprStmt& stmt) = 0;
    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    // virtual void visitIfStmt() = 0;
    virtual void visitForStmt(ForStmt& stmt) = 0;
    // virtual void visitWhileStmt() = 0;
    virtual void visitVarDeclStmt(VarDeclStmt& stmt) = 0;
    // virtual void visitFuncDeclStmt() = 0;
};

struct CodeVisitor : public ExprVisitor, public StmtVisitor {
   private:
    struct Var {
        std::string identifier;
        OptionalValue value;
    };
    std::vector<Var> m_variables;
    Player m_player;

   public:
    OptionalValue visitLiteralExpr(LiteralExpr& expr) override;
    OptionalValue visitVarExpr(VarExpr& expr) override;
    OptionalValue visitUnaryExpr(UnaryExpr& expr) override;
    OptionalValue visitBinaryExpr(BinaryExpr& expr) override;
    OptionalValue visitCallExpr(CallExpr& expr) override;

    void visitExprStmt(ExprStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
    void visitVarDeclStmt(VarDeclStmt& stmt) override;
};

class Scanner {
   private:
    std::vector<Token> m_tokens;
    Lexer m_lexer;
    size_t m_pos = -1;
    struct FunctionData {
        std::string identifier;
        size_t numberOfArguments;
    };
    std::vector<FunctionData> m_functions;

   private:
    Token& current() { return m_tokens[m_pos]; }
    Token consume() {
        Token token;
        if (++m_pos >= m_tokens.size()) {
            token = m_lexer.next();
            m_tokens.push_back(token);
        } else {
            token = m_tokens[m_pos];
        }
        return token;
    }
    Token peek() {
        Token token = consume();
        m_pos--;
        return token;
    }
    // bool accept();
    // bool expect();
    void addModifiers(CallExpr& callExpr) {
        while (peek().type == TokenType::Modifier) {
            Token token = consume();
            switch (token.text[0]) {
                case '.':
                    callExpr.inputs = token.text.substr(1);
                    break;
                case '[':
                    // TODO: Implement
                    break;
            }
        }
    }
    template <typename T, typename... Args>
    std::unique_ptr<Expr> makeExpr(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    int getPrec() { return 0; }
    std::unique_ptr<Expr> prattParse(int mininumPrecedence = 0) {
        const std::unordered_map<TokenType, int> precedence = {
            {TokenType::Add, 1},
            {TokenType::Subtract, 1},
            {TokenType::Multiply, 2},
            {TokenType::Divide, 2},
            {TokenType::Equals, 3},
            {TokenType::NotEquals, 3},
            {TokenType::LessThan, 3},
            {TokenType::GreaterThan, 3},
            {TokenType::LessThanOrEquals, 3},
            {TokenType::GreaterThanOrEquals, 3},
            {TokenType::And, 4},
            {TokenType::Or, 4},
        };
        std::unique_ptr<Expr> lhs;
        Token left = consume();
        switch (left.type) {
            case TokenType::Integer: {
                lhs = makeExpr<LiteralExpr>(LiteralExpr::Type::Integer, left.text);
                break;
            }
            case TokenType::Float: {
                lhs = makeExpr<LiteralExpr>(LiteralExpr::Type::Float, left.text);
                break;
            }
            case TokenType::Boolean: {
                lhs = makeExpr<LiteralExpr>(LiteralExpr::Type::Boolean, left.text);
                break;
            }
            case TokenType::String: {
                lhs = makeExpr<LiteralExpr>(LiteralExpr::Type::String, left.text);
                break;
            }
            case TokenType::LeftParen: {
                lhs = prattParse();
                if (consume().type != TokenType::RightParen) m_pos--;
                break;
            }
            case TokenType::Add:
            case TokenType::Subtract:
                return makeExpr<UnaryExpr>(prattParse(), left.text);
            case TokenType::Identifier: {
                if (isFunction(left)) {
                    lhs = createCallExpr();
                } else {
                    lhs = makeExpr<VarExpr>(left.text);
                }
                break;
                // TODO: Handle function calls inside expressions
                throw std::runtime_error("Function calls inside expressions are not implemented");
            }
            default:
                throw std::runtime_error("Invalid token");
        }
        while (precedence.contains(peek().type) && precedence.at(peek().type) >= mininumPrecedence) {
            Token op = consume();
            std::unique_ptr<Expr> rhs = prattParse(precedence.at(op.type) + 1);
            lhs = makeExpr<BinaryExpr>(std::move(lhs), op.text, std::move(rhs));
        }
        return lhs;
    }
    void addArguments(CallExpr& callExpr) {
        int argumentsLeft = -1;
        if (current().type == TokenType::Identifier) {
            for (auto& functionData : m_functions) {
                if (current().text == functionData.identifier) {
                    argumentsLeft = functionData.numberOfArguments;
                }
            }
        }
        Token token = peek();
        while (argumentsLeft == -1 || argumentsLeft > 0) {
            switch (token.type) {
                case TokenType::String: {
                    callExpr.arguments.push_back(makeExpr<LiteralExpr>(LiteralExpr::Type::String, consume().text));
                    break;
                }
                case TokenType::Identifier:
                case TokenType::Float:
                case TokenType::Integer:
                case TokenType::LeftParen:
                case TokenType::Add:
                case TokenType::Subtract: {
                    callExpr.arguments.push_back(prattParse());
                    break;
                }
                default:
                    return;
            }
            token = peek();
            if (argumentsLeft > 0) argumentsLeft--;
        }
    }

    std::unique_ptr<Expr> createCallExpr() {
        CallExpr callExpr;
        callExpr.identifier = current().text;
        addModifiers(callExpr);
        addArguments(callExpr);
        return std::make_unique<CallExpr>(std::move(callExpr));
    }

    bool isFunction(Token& token) {
        if (token.type == TokenType::Builtin || token.type == TokenType::Movement) return true;
        for (auto& functionData : m_functions) {
            if (token.text == functionData.identifier) return true;
        }
        return false;
    }

   public:
    BlockStmt scan() {
        BlockStmt block;
        while (consume().type != TokenType::EndOfFile && current().type != TokenType::RightBrace) {
            switch (current().type) {
                case TokenType::Builtin:
                case TokenType::Movement:
                case TokenType::Identifier: {
                    if (isFunction(current())) {
                        ExprStmt exprStmt = createCallExpr();
                        block.statements.push_back(std::make_unique<ExprStmt>(std::move(exprStmt)));
                    }
                    break;
                    // variable asignment etc
                }
                case TokenType::Let: {
                    Token token = consume();
                    if (token.type != TokenType::Identifier) throw std::runtime_error("Invalid variable name");

                    VarDeclStmt varDecl;
                    varDecl.identifier = token.text;
                    if (consume().type != TokenType::Assign) throw std::runtime_error("Expected =");
                    varDecl.value = prattParse();
                    block.statements.push_back(std::make_unique<VarDeclStmt>(std::move(varDecl)));
                    break;
                }
                case TokenType::For: {
                    ForStmt forStmt;
                    forStmt.condition = prattParse();
                    if (consume().type != TokenType::LeftBrace) throw std::runtime_error("Expected {");
                    forStmt.body = std::make_unique<BlockStmt>(scan());
                    block.statements.push_back(std::make_unique<ForStmt>(std::move(forStmt)));
                    break;
                }
                default:
                    break;
            }
        }
        return block;
    }
    Scanner(const std::string& input) : m_lexer(input) {}
};
