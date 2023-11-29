#pragma once

#include <any>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility> 

#include "Environment.hpp"
#include "Error.hpp"
#include "Expr.hpp"
#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include "LoxReturn.hpp"
#include "RuntimeError.hpp"
#include "Stmt.hpp"

// native clock added in ch10
class CLOCK: public LoxCallable {
    public:
        int arity() override { return 0; }

        std::any call(Interpreter& interpreter, std::vector<std::any> arguments) override {
            auto time = std::chrono::system_clock::now().time_since_epoch();
            return std::chrono::duration<double>{time}.count() / 1000.0;
  }

    std::string toString() override { return "<native fn>"; }
};

// updated in ch08 to include inheritance from StmtVisitor
class Interpreter: public ExprVisitor, public StmtVisitor {
    friend class LoxFunction; // added in ch10
    std::shared_ptr<Environment> globals{new Environment}; // added in ch10
    std::unordered_map<std::shared_ptr<Expr>, int> locals; // added in ch11

    public: 
        std::shared_ptr<Environment> environment = globals; // added in ch10

        void resolve (std::shared_ptr<Expr> expr, int depth) { locals[expr] = depth; } // added in ch11

        // added in ch10
        Interpreter () { globals->define("clock", std::make_shared<CLOCK>()); }

        std::any visitGroupingExpr (std::shared_ptr<Grouping> expr) override { return eval(expr->expression); } // grouping has a ref to an inner node

        std::any visitLiteralExpr (std::shared_ptr<Literal> expr) override { return expr->value; } // literal tree node -> runtime val

        // added ch09
        std::any visitLogicalExpr (std::shared_ptr<Logical> expr) override {
            std::any left = eval(expr->left);
            if (expr->op.type == Or) { if (isTruther(left)) return left; }
            else if (!isTruther(left)) return left;
            return eval(expr->right);
        }

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

        // added in ch10
        std::any visitCallExpr (std::shared_ptr<Call> expr) override {
            std::any callee = eval(expr->callee);

            std::vector<std::any> arguments;
            for (const std::shared_ptr<Expr>& argument : expr->arguments) arguments.push_back(eval(argument));

            std::shared_ptr<LoxCallable> function;
            if (callee.type() == typeid(std::shared_ptr<LoxFunction>)) function = std::any_cast<std::shared_ptr<LoxFunction>>(callee);
            else throw RuntimeError{expr->paren, "Can only call functions and classes."};

            if (arguments.size() != function->arity()) {
                throw RuntimeError{expr->paren, "Expected " +
                    std::to_string(function->arity()) + " arguments but got " +
                    std::to_string(arguments.size()) + "."};
            }

            return function->call(*this, std::move(arguments));
        }

        // void interpret (std::shared_ptr<Expr> expression) { 
        //     try {
        //         std::any value = eval(expression);
        //         std::cout << stringify(value) << std::endl;
        //     } catch (RuntimeError error) {
        //         runtimeError(error);
        //     }
        // }

        void interpret (const std::vector<std::shared_ptr<Stmt>>& statements) {
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

        // added in ch09
        // look at the condition, if it's true, execute the then branch, else execute the else branch
        std::any visitIFStmt (std::shared_ptr<IF> stmt) override {
            if (isTruther(eval(stmt->condition))) execute(stmt->thenBranch);
            else if (stmt->elseBranch != nullptr) execute(stmt->elseBranch);
            return {};
        }
        // ... added in ch08
        std::any visitPRINTStmt(std::shared_ptr<PRINT> stmt) override {
            std::any value = eval(stmt->expression);
            std::cout << stringify(value) << "\n";
            return {};
        }

        // added in ch10
        std::any visitRETURNStmt (std::shared_ptr<RETURN> stmt) override {
            std::any value = nullptr;
            if (stmt->value != nullptr) value = eval(stmt->value);

            throw LoxReturn{value};
        }

        // declaration stmts... added in ch08
        std::any visitVARStmt (std::shared_ptr<VAR> stmt) override {
            std::any value = nullptr;
            if (stmt->initializer != nullptr) value = eval(stmt->initializer);
            environment->define(stmt->name.lexeme, std::move(value));
            return {};
        }

        // added in ch09
        std::any visitWHILEStmt (std::shared_ptr<WHILE> stmt) override {
            while (isTruther(eval(stmt->condition))) execute(stmt->body);
            return {};
        }

        // evals RHS to grab val, stores val in desired var... added in ch08
        std::any visitAssignExpr (std::shared_ptr<Assign> expr) override {
            std::any value = eval(expr->value);
            // environment->assign(expr->name, value);

            auto findMe = locals.find(expr);
            if (findMe != locals.end())  {
                int distance = findMe->second;
                environment->assignAt(distance, expr->name, value);
            }
            else globals->assign(expr->name, value);

            return value;
        }

        // updated in ch11
        std::any visitVariableExpr (std::shared_ptr<Variable> expr) override { return lookUpVariable(expr->name, expr); }

        // yet another visitor ... added in ch08
         std::any visitBlockStmt(std::shared_ptr<Block> stmt) override {
            executeBlock(stmt->statements, std::make_shared<Environment>(environment));
            return {};
        }

        // added in ch10
        std::any visitFunctionStmt (std::shared_ptr<Function> stmt) override {
            // auto function = std::make_shared<LoxFunction>(stmt);
            auto function = std::make_shared<LoxFunction>(stmt, environment);
            environment->define(stmt->name.lexeme, function);
            return {};
        }

        // added in ch08... executes a list of stmts of the curr environment
        // moved inside the public scope in ch10
        void executeBlock (const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment) {
            std::shared_ptr<Environment> previous = this->environment;
            try {
                this->environment = environment;
                for (const std::shared_ptr<Stmt>& statement : statements) execute(statement);
            } catch (...) {
                this->environment = previous;
                throw;
            }

            this->environment = previous;
        }

    private:
        // std::shared_ptr<Environment> environment{new Environment}; // added in ch08 .. I moved this to the top of class

        std::any eval (std::shared_ptr<Expr> expr) { return expr->accept(*this); } // recursive helper to group

        void execute (std::shared_ptr<Stmt> stmt) { stmt->accept(*this); }

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

            if (a.type() == typeid(std::string) && b.type() == typeid(std::string)) return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
            if (a.type() == typeid(double) && b.type() == typeid(double)) return std::any_cast<double>(a) == std::any_cast<double>(b); 
            if (a.type() == typeid(bool) && b.type() == typeid(bool)) return std::any_cast<bool>(a) == std::any_cast<bool>(b);

            return false;
        }

        std::string stringify (const std::any& obj) {
            if (obj.type() == typeid(nullptr)) return "nil";

            if (obj.type() == typeid(double)) {
                std::string text = std::to_string(std::any_cast<double>(obj));
                if (text[text.length() - 2] == '.' && text[text.length() - 1] == '0') text = text.substr(0, text.length() - 2);

            return text;
            }

            if (obj.type() == typeid(std::string)) return std::any_cast<std::string>(obj);
            if (obj.type() == typeid(bool)) return std::any_cast<bool>(obj) ? "true" : "false";
            if (obj.type() == typeid(std::shared_ptr<LoxFunction>)) return std::any_cast<std::shared_ptr<LoxFunction>>(obj)->toString();

            return "Error in stringify: object type not recognized.";
        }

        void checkNumberOperands (const Token& op, const std::any& left, const std::any& right) {
            if (left.type() == typeid(double) && right.type() == typeid(double)) return;
            throw RuntimeError{op, "Operands must be numbers."};
        }

        void checkNumberOperand (const Token& op, const std::any& operand) {
            if (operand.type() == typeid(double)) return;
            throw RuntimeError{op, "Operand must be a number."};
        }

        std::any lookUpVariable (const Token& name, const std::shared_ptr<Expr>& expr) {
            auto lookMeUp = locals.find(expr);
            if (lookMeUp != locals.end()) return environment->getAt(lookMeUp->second, name.lexeme);
            else return globals->get(name);
        }
};
