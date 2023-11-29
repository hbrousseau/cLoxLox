#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Interpreter.hpp"

class Resolver: public ExprVisitor, public StmtVisitor {
    private:
        Interpreter& interpreter;
        std::vector<std::unordered_map<std::string, bool>> scopes;

        enum class FunctionType {
            NONE,
            FUNCTION
        };

        FunctionType currentFunction = FunctionType::NONE;

        void resolve (const std::shared_ptr<Stmt>& stmt) { stmt->accept(*this); }

        void resolve (const std::shared_ptr<Expr>& expr) { expr->accept(*this); }

        void endScope () { scopes.pop_back(); }

        void declare (const Token& name) {
            if (scopes.empty()) return;

            std::unordered_map<std::string, bool>& scope = scopes.back();
            if (scope.find(name.lexeme) != scope.end()) {
                error(name, "Already a variable with this name in this scope.");
            }

            scope[name.lexeme] = false; // false means declared but not defined
        }

        void define (const Token& name) {
            if (scopes.empty()) return;
            scopes.back()[name.lexeme] = true; // true means defined
        }

        void resolveLocal (const std::shared_ptr<Expr>& expr, const Token& name) {
            for (int i = scopes.size() - 1; i >= 0; --i) {
                if (scopes[i].find(name.lexeme) != scopes[i].end()) {
                    interpreter.resolve(expr, scopes.size() - 1 - i);
                    return;
                }
            }
        }

        void resolveFunction (const std::shared_ptr<Function>& function, FunctionType type) {
            FunctionType enclosingFunction = currentFunction;
            currentFunction = type;

            beginScope();
            for (const Token& param : function->params) {
                declare(param);
                define(param);
            }
            resolve(function->body);
            endScope();

            currentFunction = enclosingFunction;
        }

    public:
        Resolver (Interpreter& interpreter): interpreter(interpreter) {}

        void resolve (const std::vector<std::shared_ptr<Stmt>>& statements) {
            for (const std::shared_ptr<Stmt>& statement : statements) resolve(statement);
        }

        void beginScope () { scopes.push_back(std::unordered_map<std::string, bool>()); }

        std::any visitBlockStmt (std::shared_ptr<Block> stmt) override {
            beginScope();
            resolve(stmt->statements);
            endScope();
            return nullptr;
        }

        std::any visitVARStmt (std::shared_ptr<VAR> stmt) override {
            declare(stmt->name);
            if (stmt->initializer != nullptr) resolve(stmt->initializer);
            define(stmt->name);
            return nullptr;
        }

        std::any visitVariableExpr (std::shared_ptr<Variable> expr) override {
            if (!scopes.empty() && scopes.back().find(expr->name.lexeme) != scopes.back().end() && !scopes.back()[expr->name.lexeme]) {
                error(expr->name, "Cannot read local variable in its own initializer.");
            }

            resolveLocal(expr, expr->name);
            return {};
        }

        std::any visitAssignExpr (std::shared_ptr<Assign> expr) override {
            resolve(expr->value);
            resolveLocal(expr, expr->name);
            return {};
        }

        std::any visitFunctionStmt (std::shared_ptr<Function> stmt) override {
            declare(stmt->name);
            define(stmt->name);

            // resolveFunction(stmt);
            resolveFunction(stmt, FunctionType::FUNCTION);
            return {};
        }

        std::any visitExpressionStmt (std::shared_ptr<Expression> stmt) override {
            resolve(stmt->expression);
            return {};
        }

        std::any visitIFStmt (std::shared_ptr<IF> stmt) override {
            resolve(stmt->condition);
            resolve(stmt->thenBranch);
            if (stmt->elseBranch != nullptr) resolve(stmt->elseBranch);
            return {};
        }

        std::any visitPRINTStmt (std::shared_ptr<PRINT> stmt) override {
            resolve(stmt->expression);
            return {};
        }

        std::any visitRETURNStmt (std::shared_ptr<RETURN> stmt) override {
            if (currentFunction == FunctionType::NONE) {
                error(stmt->keyword, "Cannot return from top-level code.");
            }

            if (stmt->value != nullptr) resolve(stmt->value);

            return {};
        }

        std::any visitWHILEStmt (std::shared_ptr<WHILE> stmt) override {
            resolve(stmt->condition);
            resolve(stmt->body);
            return {};
        }

        std::any visitBinaryExpr (std::shared_ptr<Binary> expr) override {
            resolve(expr->left);
            resolve(expr->right);
            return {};
        }

        std::any visitCallExpr (std::shared_ptr<Call> expr) override {
            resolve(expr->callee);
            for (const std::shared_ptr<Expr>& argument : expr->arguments) resolve(argument);
            return {};
        }

        std::any visitGroupingExpr (std::shared_ptr<Grouping> expr) override {
            resolve(expr->expression);
            return {};
        }

        std::any visitLiteralExpr (std::shared_ptr<Literal> expr) override { return {}; }

        std::any visitLogicalExpr (std::shared_ptr<Logical> expr) override {
            resolve(expr->left);
            resolve(expr->right);
            return {};
        }

        std::any visitUnaryExpr (std::shared_ptr<Unary> expr) override {
            resolve(expr->right);
            return {};
        }
};