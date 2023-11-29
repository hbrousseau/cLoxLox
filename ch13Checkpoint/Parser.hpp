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

        // declaration    → classDecl | funDecl | varDecl | statement ;
        std::shared_ptr<Stmt> declaration() {
            try {
                if (matchMe(Class)) return classDeclaration(); // added in ch12
                if (matchMe(Fun)) return function("function");
                if (matchMe(Var)) return varDec();

                return statement();
            } catch (ParseError error) {
                sync();
                return nullptr;
            }
        }

        // classDecl      → "class" IDENTIFIER ( "<" IDENTIFIER )? "{" function* "}" ; ... updated in ch13
        std::shared_ptr<Stmt> classDeclaration() {
            Token name = consume(Identifier, "Expect class name.");

            std::shared_ptr<Variable> superclass = nullptr; // added in ch13
            if (matchMe(Less)) {
                consume(Identifier, "Expect superclass name.");
                superclass = std::make_shared<Variable>(previous());
            }

            consume(OpenBrace, "Expect '{' before class body.");

            std::vector<std::shared_ptr<Function>> methods;
            while (!check(CloseBrace) && !isAtEnd()) methods.push_back(function("method"));

            consume(CloseBrace, "Expect '}' after class body.");
           // return std::make_shared<CLASS>(std::move(name), std::move(methods)); // updated in ch13
            return std::make_shared<CLASS>(std::move(name), std::move(superclass), std::move(methods)); // added in ch13
        }

        // funDecl        → "fun" function ;
        // added in ch10
        std::shared_ptr<Function> function (std::string kind) {
            Token name = consume(Identifier, "Expect " + kind + " name.");
            consume(OpenPar, "Expect '(' after " + kind + " name.");
            std::vector<Token> parameters;
            if (!check(ClosePar)) {
                do {
                    if (parameters.size() >= 255) error(peek(), "Can't have more than 255 parameters.");
                    parameters.push_back(consume(Identifier, "Expect parameter name."));
                } while (matchMe(Comma));
            }
            consume(ClosePar, "Expect ')' after parameters.");

            consume(OpenBrace, "Expect '{' before " + kind + " body.");
            std::vector<std::shared_ptr<Stmt>> body = block();
            return std::make_shared<Function>(std::move(name), std::move(parameters), std::move(body));
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

        // assignment     → ( call "." )? IDENTIFIER "=" assignment | logic_or ; ... updated in ch12
        std::shared_ptr<Expr> assignment() {
            // std::shared_ptr<Expr> expr = equality();
            std::shared_ptr<Expr> expr = orExpr();
            if (matchMe(Equal)) {
                Token equals = previous();
                std::shared_ptr<Expr> value = assignment();

                if (Variable* express = dynamic_cast<Variable*>(expr.get())) {
                    Token name = express->name;
                    return std::make_shared<Assign>(std::move(name), value);
                }

                else if (GET* get = dynamic_cast<GET*>(expr.get())) { // added in ch12
                    return std::make_shared<SET>(get->object, std::move(get->name), value);
                }

                error(std::move(equals), "Invalid assignment target.");
            }
            return expr; 
        }

        // added in ch09
        // logic_or       → logic_and ( "or" logic_and )* ;
        std::shared_ptr<Expr> orExpr() {
            std::shared_ptr<Expr> expr = andExpr();

            while (matchMe(Or)) {
                Token op = previous();
                std::shared_ptr<Expr> right = andExpr();
                expr = std::make_shared<Logical>(expr, std::move(op), right);
            }

            return expr;
        }

        // added in ch09
        // logic_and      → equality ( "and" equality )* ;
        std::shared_ptr<Expr> andExpr() { // we eval left side first = short-circuit
            std::shared_ptr<Expr> expr = equality();

            while (matchMe(And)) {
                Token op = previous();
                std::shared_ptr<Expr> right = equality();
                expr = std::make_shared<Logical>(expr, std::move(op), right);
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

        // statement      → exprStmt | forStmt | ifStmt | printStmt | returnStmt | whileStmt | block ;
        std::shared_ptr<Stmt> statement() { // added in ch08
            if (matchMe(For)) return forStatement(); // added in ch09
            if (matchMe(If)) return ifStatement(); // added in ch09
            if (matchMe(While)) return whileStatement(); // added in ch09
            if (matchMe(Print)) return printStatement();
            if (matchMe(Return)) return returnStatement(); // added in ch10
            if (matchMe(OpenBrace)) return std::make_shared<Block>(block());
            return expressionStatement();
        }

        // returnStmt     → "return" expression? ";" ; ... added in ch10
        std::shared_ptr<Stmt> returnStatement() {
            Token keyword = previous();
            std::shared_ptr<Expr> value = nullptr;
            if (!check(Semicolon)) value = expression();

            consume(Semicolon, "Expect ';' after return value.");
            return std::make_shared<RETURN>(std::move(keyword), value);
        }

        // added in ch09
        // ifStmt         → "if" "(" expression ")" statement ( "else" statement )? ;
        std::shared_ptr<Stmt> ifStatement() {
            consume(OpenPar, "Expect '(' after 'if'.");
            std::shared_ptr<Expr> condition = expression();
            consume(ClosePar, "Expect ')' after if condition.");

            std::shared_ptr<Stmt> thenBranch = statement();
            std::shared_ptr<Stmt> elseBranch = nullptr;
            if (matchMe(Else)) elseBranch = statement();

            return std::make_shared<IF>(condition, thenBranch, elseBranch);
        }

        // added in ch09
        // whileStmt      → "while" "(" expression ")" statement ;
        std::shared_ptr<Stmt> whileStatement() {
            consume(OpenPar, "Expect '(' after 'while'.");
            std::shared_ptr<Expr> condition = expression();
            consume(ClosePar,  "Expect ')' after condition.");
            std::shared_ptr<Stmt> body = statement();

            return std::make_shared<WHILE>(condition, body);
        }

        // added in ch09
        // forStmt        → "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
        std::shared_ptr<Stmt> forStatement() {
            consume(OpenPar, "Expect '(' after 'for'.");

            // first clause = initializer
            std::shared_ptr<Stmt> initilaizer;
            if (matchMe(Semicolon)) initilaizer = nullptr;
            else if (matchMe(Var)) initilaizer = varDec();
            else initilaizer = expressionStatement();

            // next clause, condition
            std::shared_ptr<Expr> condition = nullptr;
            if (!check(Semicolon)) condition = expression();
            consume(Semicolon, "Expect ';' after loop condition");

            // last clause, increment
            std::shared_ptr<Expr> increment = nullptr;
            if (!check(ClosePar)) increment = expression();
            consume(ClosePar, "Expect ')' after for clauses.");

            // body
            std::shared_ptr<Stmt> body = statement();
            if (increment != nullptr) {
                body = std::make_shared<Block>(std::vector<std::shared_ptr<Stmt>>{body, std::make_shared<Expression>(increment)});
            }

            if (condition == nullptr) condition = std::make_shared<Literal>(true);
            body = std::make_shared<WHILE>(condition, body);

            if (initilaizer != nullptr) {
                body = std::make_shared<Block>(std::vector<std::shared_ptr<Stmt>>{initilaizer, body});
            }

            return body;
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
                // return primary();
                return call(); // updated in ch10
        }

        // call -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ; ... updated in ch12
        std::shared_ptr<Expr> call() {
            std::shared_ptr<Expr> expr = primary();

            while (true) {
                if (matchMe(OpenPar)) expr = finishCall(std::move(expr)); 
                else if (matchMe(Dot)) { // added in ch12
                    Token name = consume(Identifier, "Expect property name after '.'.");
                    expr = std::make_shared<GET>(expr, std::move(name));
                }
                else break;
            }

            return expr;
        }

        // call helper...added in ch10
        std::shared_ptr<Expr> finishCall(std::shared_ptr<Expr> callee) {
            std::vector<std::shared_ptr<Expr>> arguments;
            if (!check(ClosePar)) {
                do {
                    if (arguments.size() >= 255) error(peek(), "Can't have more than 255 arguments.");
                    arguments.push_back(expression());
                } while (matchMe(Comma));
            }

            Token paren = consume(ClosePar, "Expect ')' after arguments.");

            return std::make_shared<Call>(callee, std::move(paren), arguments);
        }

        // primary -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" | "super" "." IDENTIFIER ; ... updated in ch13
        std::shared_ptr<Expr> primary() {
            if (matchMe(False)) return std::make_shared<Literal>(false);
            if (matchMe(True)) return std::make_shared<Literal>(true);
            if (matchMe(Nil)) return std::make_shared<Literal>(nullptr);

            if (matchMe(Number, String)) return std::make_shared<Literal>(previous().literal); // added in ch08

            // added in ch13
            if (matchMe(Super)) {
                Token keyword = previous();
                consume(Dot, "Expect '.' after 'super'.");
                Token method = consume(Identifier, "Expect superclass method name.");
                return std::make_shared<SUPER>(std::move(keyword), std::move(method));
            }

            if (matchMe(This)) return std::make_shared<THIS>(previous()); // added in ch12

            if (matchMe(Identifier)) return std::make_shared<Variable>(previous());

            if (matchMe(OpenPar)) {
            std::shared_ptr<Expr> expr = expression();
            consume(ClosePar, "Expect ')' after expression.");
            return std::make_shared<Grouping>(expr);
            }

            throw error(peek(), "Expect expression.");
        }
    
};