#pragma once

#include <any>
#include <memory>
#include <utility>
#include <vector>
#include "Token.hpp"

#include "Expr.hpp"

struct Block;
struct Expression;
struct IF;
struct PRINT;
struct VAR;
struct WHILE;

struct StmtVisitor {
	virtual std::any visitBlockStmt(std::shared_ptr<Block> stmt) = 0;
	virtual std::any visitExpressionStmt(std::shared_ptr<Expression> stmt) = 0;
	virtual std::any visitIFStmt(std::shared_ptr<IF> stmt) = 0;
	virtual std::any visitPRINTStmt(std::shared_ptr<PRINT> stmt) = 0;
	virtual std::any visitVARStmt(std::shared_ptr<VAR> stmt) = 0;
	virtual std::any visitWHILEStmt(std::shared_ptr<WHILE> stmt) = 0;
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

struct IF: Stmt, public std::enable_shared_from_this<IF> {
  IF(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch)
    : condition{std::move(condition)}, thenBranch{std::move(thenBranch)}, elseBranch{std::move(elseBranch)}
  {}

	std::any accept(StmtVisitor& visitor) override {
		return visitor.visitIFStmt(shared_from_this());
  }

	const std::shared_ptr<Expr> condition;
	const std::shared_ptr<Stmt> thenBranch;
	const std::shared_ptr<Stmt> elseBranch;
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

struct WHILE: Stmt, public std::enable_shared_from_this<WHILE> {
  WHILE(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
    : condition{std::move(condition)}, body{std::move(body)}
  {}

	std::any accept(StmtVisitor& visitor) override {
		return visitor.visitWHILEStmt(shared_from_this());
  }

	const std::shared_ptr<Expr> condition;
	const std::shared_ptr<Stmt> body;
};

