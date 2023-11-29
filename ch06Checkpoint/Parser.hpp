#pragma once

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>      
#include <vector>

#include "Error.hpp"
#include "Expression.hpp"
#include "Token.hpp"
#include "TokenType.hpp"

class Parser {
    public:
        Parser (std::vector<Token>& tokens) : tokens{tokens} {} // constructor 

        std::shared_ptr<Expr> parse() {
            try {
                return expression();
            } catch (ParseError error) {
                return nullptr;
            }
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

        ParseError error (const Token& token, std::string_view message) {
            ::error(token, message);
            return ParseError{""};
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

        void sync() { // help avoid cascade errors
            advance();

            while (!isAtEnd()) {
                if (previous().type == Semicolon) return; 

                switch (peek().type) {
                    case Class:
                    case For:
                    case Fun:
                    case If:
                    case Print:
                    case Return:
                    case While:
                    case Var:
                        return;
                }
                
                advance();
            }
        }

        // expression -> equality ;
        std::shared_ptr<Expr> expression() { return equality(); }

        // equality -> comparison (("!=" | "==") comparison)* ;
        std::shared_ptr<Expr> equality() { 
            std::shared_ptr<Expr> expr = comparison(); // first nonterm in body and store result here 

            while (matchMe(BangEqual, EqualEqual)) { // ( ... )* loop... if we don't have either tokens, we must be done
                Token op = previous();
                std::shared_ptr<Expr> right = comparison();
                expr = std::make_shared<binaryExpr>(expr, std::move(op), right);
            }

            return expr;
        }

        // comparison -> term ((">" | ">=" | "<" | "<=")term)* ;
        std::shared_ptr<Expr> comparison() {
            std::shared_ptr<Expr> expr = term();

            while (matchMe(Greater, GreaterEqual, Less, LessEqual)) {
                Token op = previous();
                std::shared_ptr<Expr> right = term();
                expr = std::make_shared<binaryExpr>(expr, std::move(op), right);
            }

            return expr;
        }

        // term -> factor (("-" | "+")factor)* ;
        std::shared_ptr<Expr> term() {
            std::shared_ptr<Expr> expr = factor();

            while (matchMe(Minus, Plus)) {
                Token op = previous();
                std::shared_ptr<Expr> right = factor();
                expr = std::make_shared<binaryExpr>(expr, std::move(op), right);
            }

            return expr;
        }

        // factor -> unary(("/" | "*")unary)* ;
        std::shared_ptr<Expr> factor() {
            std::shared_ptr<Expr> expr = unary();

            while (matchMe(Slash, Star)) {
                Token op = previous();
                std::shared_ptr<Expr> right = unary();
                expr = std::make_shared<binaryExpr>(expr, std::move(op), right);
            }

            return expr; 
        }

        // unary -> ("!" | "-") unary | primary ;
        std::shared_ptr<Expr> unary() {
            if (matchMe(Bang, Minus)) {
                Token op = previous();
                std::shared_ptr<Expr> right = unary();
                return std::make_shared<unaryExpr>(std::move(op), right);
            }

            // else
            return primary();
        }

        // primary -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;
        std::shared_ptr<Expr> primary() {
            if (matchMe(False))          return std::make_shared<literalExpr>(false);
            if (matchMe(True))           return std::make_shared<literalExpr>(true);
            if (matchMe(Nil))            return std::make_shared<literalExpr>(nullptr);
            if (matchMe(Number, String)) return std::make_shared<literalExpr>(previous().literal);

            if (matchMe(OpenPar)) {
                std::shared_ptr<Expr> expr = expression();
                consume(ClosePar, "Expect ')' after expression.");
                return std::make_shared<groupingExpr>(expr);
            }

            throw error(peek(), "Expected expression.");
        }
    
};