#pragma once
#include <regex.h>

#include <memory>
#include <string>
#include <vector>

#include "lexer.h"

struct Expr {
    virtual ~Expr() = default;
    virtual void accept(struct ExprVisitor& visitor) = 0;
};

struct LiteralExpr : public Expr {
    enum class Type { Integer, Float, Boolean, String };
    Type type;
    std::string value;
    LiteralExpr(Type type, std::string value) : type(type), value(value) {}
};

struct CallExpr : public Expr {
   public:
    std::string identifier;
    std::vector<std::string> modifiers;
    std::string inputs;
    std::vector<std::unique_ptr<Expr>> arguments;
    void accept(struct ExprVisitor& visitor) override;
};

struct UnaryExpr : public Expr {};

struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> lhs;
    char operation;
    std::unique_ptr<Expr> rhs;

    explicit BinaryExpr(std::unique_ptr<Expr> lhs, char op, std::unique_ptr<Expr> rhs)
        : lhs(std::move(lhs)), operation(op), rhs(std::move(rhs)) {}
};

struct ExprVisitor {
    // virtual void visitLiteralExpr() = 0;
    virtual void visitCallExpr(CallExpr& expr) = 0;
    // virtual void visitUnaryExpr() = 0;
    // virtual void visitBinaryExpr() = 0;
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
    // virtual void visitForStmt() = 0;
    // virtual void visitWhileStmt() = 0;
    // virtual void visitVarDeclStmt() = 0;
    // virtual void visitFuncDeclStmt() = 0;
};

struct CodeVisitor : public ExprVisitor, public StmtVisitor {
    void visitCallExpr(CallExpr& expr) override;
    void visitExprStmt(ExprStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
};

class Scanner {
   private:
    std::vector<Token> tokens;
    Lexer lexer;
    size_t pos = -1;
    Token current() { return tokens[pos]; }
    Token consume() {
        Token token = lexer.next();
        if (++pos >= tokens.size()) {
            tokens.push_back(token);
        }
        return token;
    }
    Token peek() {
        Token token = consume();
        pos--;
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

   public:
    BlockStmt scan() {
        BlockStmt block;
        while (consume().type != TokenType::EndOfFile) {
            switch (current().type) {
                case TokenType::Identifier: {
                    CallExpr callExpr;
                    callExpr.identifier = current().text;
                    addModifiers(callExpr);
                    ExprStmt exprStmt(std::make_unique<CallExpr>(std::move(callExpr)));
                    block.statements.push_back(std::make_unique<ExprStmt>(std::move(exprStmt)));
                    break;
                }
                default:
                    break;
            }
        }
        return block;
    }
    Scanner(const std::string& input) : lexer(input) {}
};
