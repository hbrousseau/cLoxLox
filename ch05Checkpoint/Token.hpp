#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <any>
#include <string>
#include <utility> 
#include "TokenType.hpp"

class Token {
    public:
        const TokenType type; // represents the type of the token
        const std::string lexeme; // stores the textual rep of the token
        const std::any literal; // any is in place of java's Object ... contains the associated literal val
        const int line; // records the line number where token is located

        // token constructor
        Token (TokenType type, std::string lexeme, std::any literal, int line) 
            : type{type}, lexeme{std::move(lexeme)}, literal{std::move(literal)}, line{line} {}

        // returns a str representation of the token... examines the token's type and contructs a str based on params
        std::string toString() const {
            std::string str;

            switch (type) {
                case (False) :
                    str = "false";
                    break;
                case (Identifier) :
                    str = lexeme; 
                    break;
                case (Number) :
                    // For tokens of type Number, convert the literal value to a string and store it
                    str = std::to_string(std::any_cast<double>(literal));
                    break;
                case (String) :
                    // For tokens of type String, extract the string literal from the 'literal' member
                    str = std::any_cast<std::string>(literal);
                    break;
                case (True) :
                    str = "true"; 
                    break;
                default : 
                    str = "nil"; // for other token types, assume nil
                    break;
            }

            // Return the formatted string representation, including the token type, lexeme, and content.
            return ::tokensToString(type) + " " + lexeme + " " + str;
        }
};

#endif 