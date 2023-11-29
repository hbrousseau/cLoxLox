#pragma once

#include <any>
#include <memory>
#include <utility>
#include <vector>
#include "Token.hpp"

#include "Expr.hpp"

struct Block;
struct Expression;
struct PRINT;
struct VAR;

struct StmtVisitor {
	virtual std::any visitBlockStmt(std::shared_ptr<Block> stmt) = 0;
	virtual std::any visitExpressionStmt(std::shared_ptr<Expression> stmt) = 0;
	virtual std::any visitPRINTStmt(std::shared_ptr<PRINT> stmt) = 0;
	virtual std::any visitVARStmt(std::shared_ptr<VAR> stmt) = 0;
	virtual ~StmtVisitor() = default;
};

struct Stmt {
	virtual std::any accept(StmtVisitor& visitor) = 0;
};

struct Block: Stmt, public std::enable_shared_from_this<Block> {
  Block(std::vector<std::shared_ptr<Stmt>> statements)
    : statements{std::move(statements)}
  {}

	std::any accept(StmtVisitor& visitor) override {
		return visitor.visitBlockStmt(shared_from_this());
  }

	const std::vector<std::shared_ptr<Stmt>> statements;
};

struct Expression: Stmt, public std::enable_shared_from_this<Expression> {
  Expression(std::shared_ptr<Expr> expression)
    : expression{std::move(expression)}
  {}

	std::any accept(StmtVisitor& visitor) override {
		return visitor.visitExpressionStmt(shared_from_this());
  }

	const std::shared_ptr<Expr> expression;
};

struct PRINT: Stmt, public std::enable_shared_from_this<PRINT> {
  PRINT(std::shared_ptr<Expr> expression)
    : expression{std::move(expression)}
  {}

	std::any accept(StmtVisitor& visitor) override {
		return visitor.visitPRINTStmt(shared_from_this());
  }

	const std::shared_ptr<Expr> expression;
};

struct VAR: Stmt, public std::enable_shared_from_this<VAR> {
  VAR(Token name, std::shared_ptr<Expr> initializer)
    : name{std::move(name)}, initializer{std::move(initializer)}
  {}

	std::any accept(StmtVisitor& visitor) override {
		return visitor.visitVARStmt(shared_from_this());
  }

	const Token name;
	const std::shared_ptr<Expr> initializer;
};

