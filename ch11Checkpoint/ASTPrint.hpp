#pragma once

#include <any>
#include <cassert>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>

#include "Expr.hpp"

// note: In ch08, I realized that I had some boo boos. Instead of completely restarting, I am going to make the edits within this chapter
// the main issues were in my GenerateAST.cpp and Parser.cpp as I am really good at skipping code and not reading directions fully :)

class ASTPrinter : public ExprVisitor {
    private:
        template <class... EXPR>
        std::string addParentheses (std::string_view name, EXPR... exprs) {
            assert((... && std::is_same_v<EXPR, std::shared_ptr<Expr>>));
            std::ostringstream frankenstr;

            frankenstr << "(" << name;
            ((frankenstr << " " << print(exprs)), ...);
            frankenstr << ")";
            return frankenstr.str();
        }

    public: 
        std::string print (std::shared_ptr<Expr> expr) { return std::any_cast<std::string>(expr->accept(*this)); }

        std::any visitBinaryExpr (std::shared_ptr<Binary> expr) override { return addParentheses(expr->op.lexeme, expr->left, expr->right); }

        std::any visitGroupingExpr (std::shared_ptr<Grouping> expr) override { return addParentheses("group", expr->expression); }

        std::any visitLiteralExpr (std::shared_ptr<Literal> expr) override { 
            auto& valType = expr->value.type();
            if (valType == typeid(nullptr)) return "nil";
            else if (valType == typeid(std::string))  return std::any_cast<std::string>(expr->value);
            else if (valType == typeid(double)) return std::to_string(std::any_cast<double>(expr->value));
            else if (valType == typeid(bool)) return std::any_cast<bool>(expr->value) ? "true" : "false";

            return "Error in literalVisitor override: literal type not recognized.";
        }

        std::any visitUnaryExpr (std::shared_ptr<Unary> expr) override { return addParentheses(expr->op.lexeme, expr->right); }
};