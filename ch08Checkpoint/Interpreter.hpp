#pragma once

#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "Environment.hpp"
#include "Error.hpp"
#include "Expr.hpp"
#include "RuntimeError.hpp"
#include "Stmt.hpp"

// updated in ch08 to include inheritance from StmtVisitor
class Interpreter: public ExprVisitor, public StmtVisitor {
    public: 
        std::any visitLiteralExpr (std::shared_ptr<Literal> expr) override { return expr->value; } // literal tree node -> runtime val

        std::any visitGroupingExpr (std::shared_ptr<Grouping> expr) override { return eval(expr->expression); } // grouping has a ref to an inner node

        std::any visitUnaryExpr (std::shared_ptr<Unary> expr) override {
            std::any right = eval(expr->right); // eval operand expr

            switch (expr->op.type) { // apply unary op to result ... post-order trav
                case Bang  :
                    return !isTruther(right); // Lox -> false && nill = false, rest = true
                case Minus : 
                    checkNumberOperand(expr->op, right);
                    return -std::any_cast<double>(right);
            }

            // Unreachable
            return {}; // compiler will determine type
        }

        std::any visitBinaryExpr (std::shared_ptr<Binary> expr) {
            std::any left  = eval(expr->left);
            std::any right = eval(expr->right);

            switch (expr->op.type) {
                case Greater      :
                    checkNumberOperands(expr->op, left, right);
                    return std::any_cast<double>(left) >  std::any_cast<double>(right);
                case GreaterEqual :
                    checkNumberOperands(expr->op, left, right);
                    return std::any_cast<double>(left) >= std::any_cast<double>(right);
                case Less         :
                    checkNumberOperands(expr->op, left, right);
                    return std::any_cast<double>(left) <  std::any_cast<double>(right);
                case LessEqual    :
                    checkNumberOperands(expr->op, left, right);
                    return std::any_cast<double>(left) <= std::any_cast<double>(right);
                case BangEqual    : return !isEqual(left, right);
                case EqualEqual   : return isEqual (left, right);
                case Minus        :
                    checkNumberOperands(expr->op, left, right);;
                    return std::any_cast<double>(left) -  std::any_cast<double>(right);
                case Slash        : 
                    checkNumberOperands(expr->op, left, right);
                    return std::any_cast<double>(left) /  std::any_cast<double>(right);
                case Star         :
                    checkNumberOperands(expr->op, left, right);
                    return std::any_cast<double>(left) *  std::any_cast<double>(right);
                case Plus         :  
                    if (left.type() == typeid(double) && right.type() == typeid(double)) { // both are numbers, addition
                        return std::any_cast<double>(left) + std::any_cast<double>(right);
                    }

                    if (left.type() == typeid(std::string) && right.type() == typeid(std::string)) { // both are strings, concatenate
                        return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
                    }

                    // break; 
                    throw RuntimeError{expr->op, "Operands must be two numbers or two strings."};
            }

            // unreachable
            return {};
        }

        // void interpret (std::shared_ptr<Expr> expression) { 
        //     try {
        //         std::any value = eval(expression);
        //         std::cout << stringify(value) << std::endl;
        //     } catch (RuntimeError error) {
        //         runtimeError(error);
        //     }
        // }

        void interpret (const std::vector<std::shared_ptr<Stmt>>& statements) { // new and improved as of ch08
            try {
                for (const std::shared_ptr<Stmt>& statement : statements) execute(statement);
            } catch (RuntimeError error) {
                runtimeError(error);
            }
        }

        // evaluate the inner expression using eval() and discard the val ... added in ch08
        std::any visitExpressionStmt (std::shared_ptr<Expression> stmt) override {
            eval(stmt->expression);
            return {};
        }

        // ... added in ch08
        std::any visitPRINTStmt (std::shared_ptr<PRINT> stmt) override {
            std::any value = eval(stmt->expression);
            std::cout << stringify(value) << std::endl;
            return {};
        }

        // declaration stmts... added in ch08
        std::any visitVARStmt (std::shared_ptr<VAR> stmt) override {
            std::any value = nullptr;
            if (stmt->initializer != nullptr) value = eval(stmt->initializer);

            environment->define(stmt->name.lexeme, std::move(value));
            return {};
        }

        // evals RHS to grab val, stores val in desired var... added in ch08
        std::any visitAssignExpr (std::shared_ptr<Assign> expr) override {
            std::any value = eval(expr->value);
            environment->assign(expr->name, value);
            return value;
        }

        // evaluate var expr... added in ch08
        std::any visitVariableExpr (std::shared_ptr<Variable> expr) override { return environment->get(expr->name); }

        // yet another visitor ... added in ch08
        std::any visitBlockStmt (std::shared_ptr<Block> stmt) override {
            executeBlock(stmt->statements, std::make_shared<Environment>(environment));
            return {};
        }

    private:
        std::shared_ptr<Environment> environment{new Environment}; // added in ch08

        std::any eval (std::shared_ptr<Expr> expr) { return expr->accept(*this); } // recursive helper to group

        void execute (std::shared_ptr<Stmt> stmt) { stmt->accept(*this); }

        // added in ch08... executes a list of stmts of the curr environment
        void executeBlock (const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment) {
            std::shared_ptr<Environment> prev = this->environment;
            try {
                this->environment = environment;
                for (const std::shared_ptr<Stmt>& statement : statements) execute(statement);
            } catch (...) {
                this->environment = prev;
                throw;
            }

            this->environment = prev;
        }

        bool isTruther (const std::any& obj) {
            // I ain't calling you a truther 
            if (obj.type() == typeid(nullptr)) return false;
            if (obj.type() == typeid(bool)) return std::any_cast<bool>(obj);
            // okay, you're a truther
            return true;
        }

        bool isEqual (const std::any& a, const std::any& b) {
            if (a.type() == typeid(nullptr) && b.type() == typeid(nullptr)) return true;
            if (a.type() == typeid(nullptr)) return false; 

            // return a.equals(b);
            // check other types and return T/F based on ret statement
            if (a.type() == typeid(bool) && b.type() == typeid(bool)) return std::any_cast<bool>(a) == std::any_cast<bool>(b); 
            if (a.type() == typeid(double) && b.type() == typeid(double)) return std::any_cast<double>(a) == std::any_cast<double>(b);
            if (a.type() == typeid(std::string) && b.type() == typeid(std::string)) return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);

            return false; // aslo applies to b.type() == typeid(nullptr)
        }

        std::string stringify (const std::any& obj) {
            if (obj.type() == typeid(nullptr)) return "nil";
            
            if (obj.type() == typeid(double)) {
                std::string text = std::to_string(std::any_cast<double>(obj));
                if (text[text.length() - 2] == '.' && text[text.length() - 1] == '0') {
                    text = text.substr(0, text.length() - 2);
                }
                return text;
            }
            // return obj.toString()
            if (obj.type() == typeid(std::string)) return std::any_cast<std::string>(obj);
            if (obj.type() == typeid(bool)) return std::any_cast<bool>(obj) ? "true" : "false";

            return "Stringify error: object type not recognized.";
        }

        void checkNumberOperand (const Token& op, const std::any& operand) {
            if (operand.type() == typeid(double)) return;
            throw RuntimeError{op, "Operand must be a number."};
        }

        void checkNumberOperands (const Token& op, const std::any& left, const std::any& right) {
            if (left.type() == typeid(double) && right.type() == typeid(double)) return; 
            throw RuntimeError{op, "Operands must be numbers."};
        }

};
