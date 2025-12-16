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

struct AssignExpr : public Expr {
    std::string identifier;
    std::unique_ptr<Expr> value;
    OptionalValue accept(struct ExprVisitor& visitor) override;
    AssignExpr(std::string identifier, std::unique_ptr<Expr> value) : identifier(identifier), value(std::move(value)) {}
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
    virtual OptionalValue visitAssignExpr(AssignExpr& expr) = 0;
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
    ExprStmt() = default;
    ExprStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
};

struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    bool tap = false;
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
    std::string identifier;
    std::vector<std::string> parameters;
    std::unique_ptr<Stmt> body;
    void accept(struct StmtVisitor& visitor) override;
};

struct StmtVisitor {
    virtual void visitExprStmt(ExprStmt& stmt) = 0;
    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    virtual void visitIfStmt(IfStmt& stmt) = 0;
    virtual void visitForStmt(ForStmt& stmt) = 0;
    virtual void visitWhileStmt(WhileStmt& stmt) = 0;
    virtual void visitVarDeclStmt(VarDeclStmt& stmt) = 0;
    virtual void visitFuncDeclStmt(FuncDeclStmt& stmt) = 0;
};

struct CodeVisitor : public ExprVisitor, public StmtVisitor {
   private:
    struct Var {
        std::string identifier;
        OptionalValue value;
    };
    bool m_tap = false;
    std::vector<Var> m_variables;
    std::vector<FuncDeclStmt> m_functions;
    Player m_player;

   public:
    OptionalValue visitLiteralExpr(LiteralExpr& expr) override;
    OptionalValue visitVarExpr(VarExpr& expr) override;
    OptionalValue visitAssignExpr(AssignExpr& expr) override;
    OptionalValue visitUnaryExpr(UnaryExpr& expr) override;
    OptionalValue visitBinaryExpr(BinaryExpr& expr) override;
    OptionalValue visitCallExpr(CallExpr& expr) override;

    void visitExprStmt(ExprStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
    void visitWhileStmt(WhileStmt& stmt) override;
    void visitVarDeclStmt(VarDeclStmt& stmt) override;
    void visitFuncDeclStmt(FuncDeclStmt& stmt) override;
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
        // Increasing order of precedence
        const std::unordered_map<TokenType, int> precedence = {
            {TokenType::Or, 1},
            {TokenType::And, 2},
            {TokenType::Equals, 3},
            {TokenType::NotEquals, 3},
            {TokenType::LessThan, 4},
            {TokenType::GreaterThan, 4},
            {TokenType::LessThanOrEquals, 4},
            {TokenType::GreaterThanOrEquals, 4},
            {TokenType::Add, 5},
            {TokenType::Subtract, 5},
            {TokenType::Multiply, 6},
            {TokenType::Divide, 6},
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
                lhs = makeExpr<UnaryExpr>(prattParse(10), left.text);
                break;
            case TokenType::Identifier: {
                if (isFunction(left)) {
                    lhs = createCallExpr();
                } else if (peek().type == TokenType::Assign) {
                    consume();
                    lhs = makeExpr<AssignExpr>(left.text, prattParse());
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
                case TokenType::String:
                case TokenType::Boolean:
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

    std::unique_ptr<Stmt> parseStmt() {
        switch (current().type) {
            case TokenType::Builtin:
            case TokenType::Movement:
            case TokenType::Identifier: {
                ExprStmt exprStmt;
                if (isFunction(current())) {
                    exprStmt = createCallExpr();
                } else {
                    m_pos--;
                    exprStmt = prattParse();
                }
                return std::make_unique<ExprStmt>(std::move(exprStmt));
            }
            case TokenType::Let: {
                Token token = consume();
                if (token.type != TokenType::Identifier) throw std::runtime_error("Invalid variable name");

                VarDeclStmt varDecl;
                varDecl.identifier = token.text;
                if (consume().type != TokenType::Assign) throw std::runtime_error("Expected =");
                varDecl.value = prattParse();
                return std::make_unique<VarDeclStmt>(std::move(varDecl));
            }
            case TokenType::FuncDecl: {
                Token token = consume();
                if (token.type != TokenType::Identifier) throw std::runtime_error("Invalid function name");

                FuncDeclStmt funcDecl;
                funcDecl.identifier = token.text;

                token = consume();
                if (token.type != TokenType::LeftParen) throw std::runtime_error("Expected (");
                while (consume().type == TokenType::Identifier) funcDecl.parameters.push_back(current().text);
                if (current().type == TokenType::RightParen) consume();
                if (current().type == TokenType::Movement || current().type == TokenType::Builtin)
                    throw std::runtime_error("Can't override builtin functions");

                funcDecl.body = parseStmt();
                m_functions.push_back(FunctionData{funcDecl.identifier, funcDecl.parameters.size()});
                return std::make_unique<FuncDeclStmt>(std::move(funcDecl));
            }
            case TokenType::For: {
                ForStmt forStmt;
                forStmt.condition = prattParse();
                consume();
                forStmt.body = parseStmt();
                return std::make_unique<ForStmt>(std::move(forStmt));
            }
            case TokenType::While: {
                WhileStmt whileStmt;
                whileStmt.condition = prattParse();
                consume();
                whileStmt.body = parseStmt();
                return std::make_unique<WhileStmt>(std::move(whileStmt));
            }
            case TokenType::If: {
                IfStmt ifStmt;
                ifStmt.condition = prattParse();
                consume();
                ifStmt.thenBranch = parseStmt();
                if (peek().type == TokenType::Else) {
                    consume();
                    consume();
                    ifStmt.elseBranch = parseStmt();
                }
                return std::make_unique<IfStmt>(std::move(ifStmt));
            }
            case TokenType::Tap: {
                if (peek().type == TokenType::LeftBrace) consume();
                BlockStmt stmt = scan();
                stmt.tap = true;
                return std::make_unique<BlockStmt>(std::move(stmt));
            }
            case TokenType::LeftBrace: {
                return std::make_unique<BlockStmt>(scan());
            }
            case TokenType::Semicolon: {
                if (consume().type == TokenType::RightBrace) return std::unique_ptr<Stmt>();
                return parseStmt();
            }
            case TokenType::EndOfFile: {
                return std::unique_ptr<Stmt>();
            }
            default:
                break;
        }
        throw std::runtime_error("Error while parsing statement: " + current().text);
    }

   public:
    BlockStmt scan() {
        BlockStmt block;
        while (consume().type != TokenType::EndOfFile && current().type != TokenType::RightBrace) {
            std::unique_ptr<Stmt> stmt = parseStmt();
            if (stmt) {
                block.statements.push_back(std::move(stmt));
            } else {
                return block;
            }
        }
        return block;
    }
    Scanner(const std::string& input) : m_lexer(input) {}
};
