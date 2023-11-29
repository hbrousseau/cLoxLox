#pragma once

#include <any>
#include <cassert>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>

#include "Expression.hpp"

class ASTPrinter : public visitingExpr {
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

        std::any binaryVisitor (std::shared_ptr<binaryExpr> expr) override { return addParentheses(expr->operation.lexeme, expr->LHS, expr->RHS); }

        std::any groupVisitor (std::shared_ptr<groupingExpr> expr) override { return addParentheses("group", expr->expr); }

        std::any literalVisitor (std::shared_ptr<literalExpr> expr) override { 
            auto& valType = expr->value.type();
            if (valType == typeid(nullptr)) return "nil";
            else if (valType == typeid(std::string))  return std::any_cast<std::string>(expr->value);
            else if (valType == typeid(double)) return std::to_string(std::any_cast<double>(expr->value));
            else if (valType == typeid(bool)) return std::any_cast<bool>(expr->value) ? "true" : "false";

            return "Error in literalVisitor override: literal type not recognized.";
        }

        std::any unaryVisitor (std::shared_ptr<unaryExpr> expr) override { return addParentheses(expr->operation.lexeme, expr->RHS); }
};