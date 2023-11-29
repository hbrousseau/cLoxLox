#pragma once

#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "Error.hpp"
#include "Expression.hpp"
#include "RuntimeError.hpp"

// class declares that it's a visitor
class Interpreter : public visitingExpr {
    public: 
        std::any literalVisitor (std::shared_ptr<literalExpr> expr) override { return expr->value; } // literal tree node -> runtime val

        std::any groupVisitor (std::shared_ptr<groupingExpr> expr) override { return eval(expr->expr); } // grouping has a ref to an inner node

        std::any unaryVisitor (std::shared_ptr<unaryExpr> expr) override {
            std::any right = eval(expr->RHS); // eval operand expr

            switch (expr->operation.type) { // apply unary op to result ... post-order trav
                case Bang  :
                    return !isTruther(right); // Lox -> false && nill = false, rest = true
                case Minus : 
                    checkNumberOperand(expr->operation, right);
                    return -std::any_cast<double>(right);
            }

            // Unreachable
            return {}; // compiler will determine type
        }

        std::any binaryVisitor (std::shared_ptr<binaryExpr> expr) {
            std::any left  = eval(expr->LHS);
            std::any right = eval(expr->RHS);

            switch (expr->operation.type) {
                case Greater      :
                    checkNumberOperands(expr->operation, left, right);
                    return std::any_cast<double>(left) >  std::any_cast<double>(right);
                case GreaterEqual :
                    checkNumberOperands(expr->operation, left, right);
                    return std::any_cast<double>(left) >= std::any_cast<double>(right);
                case Less         :
                    checkNumberOperands(expr->operation, left, right);
                    return std::any_cast<double>(left) <  std::any_cast<double>(right);
                case LessEqual    :
                    checkNumberOperands(expr->operation, left, right);
                    return std::any_cast<double>(left) <= std::any_cast<double>(right);
                case BangEqual    : return !isEqual(left, right);
                case EqualEqual   : return isEqual (left, right);
                case Minus        :
                    checkNumberOperands(expr->operation, left, right);;
                    return std::any_cast<double>(left) -  std::any_cast<double>(right);
                case Slash        : 
                    checkNumberOperands(expr->operation, left, right);
                    return std::any_cast<double>(left) /  std::any_cast<double>(right);
                case Star         :
                    checkNumberOperands(expr->operation, left, right);
                    return std::any_cast<double>(left) *  std::any_cast<double>(right);
                case Plus         :  
                    if (left.type() == typeid(double) && right.type() == typeid(double)) { // both are numbers, addition
                        return std::any_cast<double>(left) + std::any_cast<double>(right);
                    }

                    if (left.type() == typeid(std::string) && right.type() == typeid(std::string)) { // both are strings, concatenate
                        return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
                    }

                    // break; 
                    throw RuntimeError{expr->operation, "Operands must be two numbers or two strings."};
            }

            // unreachable
            return {};
        }

        void interpret (std::shared_ptr<Expr> expression) {
            try {
                std::any value = eval(expression);
                std::cout << stringify(value) << std::endl;
            } catch (RuntimeError error) {
                runtimeError(error);
            }
        }

    private:
        std::any eval (std::shared_ptr<Expr> expr) { return expr->accept(*this); } // recursive helper to group

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