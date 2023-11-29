#pragma once

#include <any>
#include <cassert>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>

#include "Expression.hpp"
#include "Token.hpp"

class ASTPrinter : public visitingExpr {
    private:
        template <class... expr>
        std::string addParentheses (std::string_view name, expr... exprs) {
            assert((... && std::is_same_v < expr, std::shared_ptr<Expr>>));
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

        std::any literalVisitor (std::shared_ptr<literalExpr> expr) override { return expr->toString(); }

        std::any unaryVisitor (std::shared_ptr<unaryExpr> expr) override { return addParentheses(expr->operation.lexeme, expr->RHS); }
};