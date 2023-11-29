#pragma once

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>      
#include <vector>

#include "Error.hpp"
#include "Expr.hpp"
#include "Stmt.hpp" // added in ch08
#include "Token.hpp"
#include "TokenType.hpp"

class Parser {
    public:
        Parser (std::vector<Token>& tokens) : tokens{tokens} {} // constructor 

        // std::shared_ptr<Expr> parse() { // this temporarily allowed ch06 to work, now in ch08 we update
        //     try {
        //         return expression();
        //     } catch (ParseError error) {
        //         return nullptr;
        //     }
        // }

        std::vector<std::shared_ptr<Stmt>> parse() { // new parse fun from ch08
            std::vector<std::shared_ptr<Stmt>> statements;
            while (!isAtEnd()) {
                // statements.push_back(statement());
                statements.push_back(declaration()); 
            }

            return statements;
        }

    private: 
        const std::vector<Token>& tokens; 
        int curr = 0; // used to point to next tokens
                
        class ParseError: public std::runtime_error {
            public:
                using std::runtime_error::runtime_error;
        };

        Token advance() {
            if (!isAtEnd()) curr++; // advance 
            return previous();
        }

        bool check (TokenType type) {
            if (isAtEnd()) return false;
            return peek().type == type; 
        }

        Token consume (TokenType type, std::string_view message) {
            if (check(type)) return advance(); // check to see if next token is of the expected type
            throw error(peek(), message);
        }

        // declaration    → varDecl | statement ;
        std::shared_ptr<Stmt> declaration() {
            try {
                if (matchMe(Var)) return varDec();
                return statement();
            } catch (ParseError error) {
                sync();
                return nullptr;
            }
        }

        ParseError error (const Token& token, std::string_view message) {
            ::error(token, message);
            return ParseError{""};
        }
        
        std::shared_ptr<Stmt> expressionStatement() { 
            std::shared_ptr<Expr> expr = expression(); 
            consume(Semicolon, "Expect ';' after expression.");
            return std::make_shared<Expression>(expr); 
        }

        // added in ch08
        std::shared_ptr<Expr> assignment() {
            std::shared_ptr<Expr> expr = equality();
            if (matchMe(Equal)) {
                Token equals = previous();
                std::shared_ptr<Expr> value = assignment();

                if (Variable* express = dynamic_cast<Variable*>(expr.get())) {
                    Token name = express->name;
                    return std::make_shared<Assign>(std::move(name), value);
                }

                error(std::move(equals), "Invalid assignment target.");
            }
            return expr; 
        }
        
        // added in ch08
        std::vector<std::shared_ptr<Stmt>> block() {
            std::vector<std::shared_ptr<Stmt>> statements;
            while (!check(CloseBrace) && !isAtEnd()) statements.push_back(declaration());
            
            consume(CloseBrace, "Expect '}' after block.");
            return statements;
        }

        bool isAtEnd() { return peek().type == Eof; } // run out of tokens?

        template <class... Types>
        bool matchMe (Types... type) { // check to see if curr token has any of the given types... if true, consume token and return false
            assert((... && std::is_same_v<Types, TokenType>));  // Ensure that all types match TokenTypes

            for (auto type : {type...}) { // loop through all types to find a match
                if (check(type)) {
                    advance();
                    return true;
                }
            }

            return false;
        }

        Token peek() { return tokens.at(curr); } // returns next token to consume

        Token previous() { return tokens.at(curr - 1); } // returns most recently consumed token

        std::shared_ptr<Stmt> printStatement() {
            std::shared_ptr<Expr> value = expression();
            consume(Semicolon, "Expect ';' after value.");
            return std::make_shared<PRINT>(value);
        }

        // statement      → exprStmt | printStmt ;
        std::shared_ptr<Stmt> statement() { // added in ch08
            if (matchMe(Print)) return printStatement();
            if (matchMe(OpenBrace)) return std::make_shared<Block>(block());
            return expressionStatement();
        }

        void sync() { // help avoid cascade errors
            advance();

            while (!isAtEnd()) {
            if (previous().type == Semicolon) return;

            switch (peek().type) {
                case Class:
                case Fun:
                case Var:
                case For:
                case If:
                case While:
                case Print:
                case Return:
                return;
            }

            advance();
            }
        } 

        // varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
        std::shared_ptr<Stmt> varDec() {
            Token name = consume(Identifier, "Expect variable name.");

            std::shared_ptr<Expr> initializer = nullptr;
            if (matchMe(Equal)) initializer = expression();

            consume(Semicolon, "Expect ';' after variable declaration.");
            return std::make_shared<VAR>(std::move(name), initializer);
        }

        // expression -> equality ;
        // std::shared_ptr<Expr> expression() { return equality(); } ... modified in ch08
        std::shared_ptr<Expr> expression() { return assignment(); } 

        // equality -> comparison (("!=" | "==") comparison)* ;
        std::shared_ptr<Expr> equality() { 
            std::shared_ptr<Expr> expr = comparison();

            while (matchMe(BangEqual, EqualEqual)) {
            Token op = previous();
            std::shared_ptr<Expr> right = comparison();
            expr = std::make_shared<Binary>(expr, std::move(op), right);
            }

            return expr;
        }

        // comparison -> term ((">" | ">=" | "<" | "<=")term)* ;
        std::shared_ptr<Expr> comparison() {
            std::shared_ptr<Expr> expr = term();

            while (matchMe(Greater, GreaterEqual, Less, LessEqual)) {
            Token op = previous();
            std::shared_ptr<Expr> right = term();
            expr = std::make_shared<Binary>(expr, std::move(op), right);
            }

            return expr;
        }

        // term -> factor (("-" | "+")factor)* ;
        std::shared_ptr<Expr> term() {
            std::shared_ptr<Expr> expr = factor();

            while (matchMe(Minus, Plus)) {
            Token op = previous();
            std::shared_ptr<Expr> right = factor();
            expr = std::make_shared<Binary>(expr, std::move(op), right);
            }

            return expr;
        }

        // factor -> unary(("/" | "*")unary)* ;
        std::shared_ptr<Expr> factor() {
            std::shared_ptr<Expr> expr = unary();

            while (matchMe(Slash, Star)) {
            Token op = previous();
            std::shared_ptr<Expr> right = unary();
            expr = std::make_shared<Binary>(expr, std::move(op), right);
            }

            return expr;
        }

        // unary -> ("!" | "-") unary | primary ;
        std::shared_ptr<Expr> unary() {
                if (matchMe(Bang, Minus)) {
                Token op = previous();
                std::shared_ptr<Expr> right = unary();
                return std::make_shared<Unary>(std::move(op), right);
                }
                // | primary
                return primary();
        }

        // primary -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;
        std::shared_ptr<Expr> primary() {
            if (matchMe(False)) return std::make_shared<Literal>(false);
            if (matchMe(True)) return std::make_shared<Literal>(true);
            if (matchMe(Nil)) return std::make_shared<Literal>(nullptr);

            if (matchMe(Number, String)) return std::make_shared<Literal>(previous().literal); // added in ch08

            if (matchMe(Identifier)) return std::make_shared<Variable>(previous());

            if (matchMe(OpenPar)) {
            std::shared_ptr<Expr> expr = expression();
            consume(ClosePar, "Expect ')' after expression.");
            return std::make_shared<Grouping>(expr);
            }

            throw error(peek(), "Expect expression.");
        }
    
};