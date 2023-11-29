#pragma once

// this is an expr tree structure 

#include <any>
#include <memory>
#include <typeinfo>
#include <utility>
#include <vector>

#include "Token.hpp"

// forward declarations of the classes to be defined later
class binaryExpr;
class Expr;
class groupingExpr;
class literalExpr;
class unaryExpr;
class visitingExpr;

// base visitor interface for Expr types
class visitingExpr {
    public:
        virtual std::any binaryVisitor(std::shared_ptr<binaryExpr> expr) = 0;
        virtual std::any groupVisitor(std::shared_ptr<groupingExpr> expr) = 0;
        virtual std::any literalVisitor(std::shared_ptr<literalExpr> expr) = 0;
        virtual std::any unaryVisitor(std::shared_ptr<unaryExpr> expr) = 0;
        virtual ~visitingExpr() = default; // default destructor
};

// base class for any expr type which the rest is inherited from
class Expr { 
    public:
        virtual std::any accept(visitingExpr& visitor) = 0; 
}; // accepts visitors 

// binary Expr class and use enable_shared_from_this to prevent premature distruction of shared objs
class binaryExpr : public Expr, public std::enable_shared_from_this<binaryExpr> {
    public:
        const std::shared_ptr<Expr> LHS, RHS; // left and right hand sides
        const Token operation;

        binaryExpr (std::shared_ptr<Expr> LHS, Token operation, std::shared_ptr<Expr> RHS) 
            : LHS{std::move(LHS)}, operation{std::move(operation)}, RHS{std::move(RHS)} {} // constructor binaryExpr = LHS operator RHS
        
         // this and subsequent accept overrides allows a call to the appropriate visitingExpr function without dynamic casting or type checks
        std::any accept(visitingExpr& visitor) override { return visitor.binaryVisitor(shared_from_this()); }
};

// grouping Expr class
class groupingExpr : public Expr, public std::enable_shared_from_this<groupingExpr> {
    public:
        const std::shared_ptr<Expr> expr;

        groupingExpr (std::shared_ptr<Expr> expr) : expr{std::move(expr)} {} // constructor
        
        std::any accept (visitingExpr& visitor) override { return visitor.groupVisitor(shared_from_this()); }
};

// literal Expr class
class literalExpr : public Expr, public std::enable_shared_from_this<literalExpr> {
    public:
        const std::any value;

        literalExpr (std::any value) : value{std::move(value)} {} // constructor

        std::any accept (visitingExpr& visitor) override { return visitor.literalVisitor(shared_from_this()); }
};

// unary Expr class
class unaryExpr : public Expr, public std::enable_shared_from_this<unaryExpr> {
    public: 
        const Token operation;
        const std::shared_ptr<Expr> RHS;

        unaryExpr (Token operation, std::shared_ptr<Expr> RHS) 
            : operation{std::move(operation)}, RHS{std::move(RHS)} {} // constructor

        std::any accept (visitingExpr& visitor) override { return visitor.unaryVisitor(shared_from_this()); }
};