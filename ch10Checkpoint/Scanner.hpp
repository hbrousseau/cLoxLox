#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Error.hpp"
#include "Token.hpp"

class Scanner {
    public:
        Scanner (std::string_view src) : src{src} {} // constructor

        std::vector<Token> scanTokens() {
            while (!isAtEnd()) {
                // we are at the beginning of the next lexeme
                start = curr;
                tokenScan();
            }

            tokens.emplace_back(Eof, "", nullptr, line);
            return tokens;
        }
    
    private:
        std::string_view src;
        std::vector<Token> tokens;
        static const std::unordered_map<std::string, TokenType> keywords;
        int curr = 0, line = 1, start = 0;

        bool isAtEnd() { return curr >= src.length(); }
        
        bool isDigit (char isMe) { return isMe >= '0' && isMe <= '9'; } // if between 0 and 9 isMe is indeed a digit

        bool isAlpha (char isMe) { return (isMe >= 'a' && isMe <= 'z') || (isMe >= 'A' && isMe <= 'Z') || (isMe == '_'); } // upper/lower case and underscore = isAlpha

        bool isAlphaNumeric (char isMe) { return isAlpha(isMe) || isDigit(isMe); }

        bool matchMe(char me) {
            if (isAtEnd()) return false;
            if (src.at(curr) != me) return false;
            
            curr++;
            return true;
        }
        
        char advance() { return src.at(curr++); }

        char peek() {
            if (isAtEnd()) return '\0'; // return null term
            return src.at(curr);
        }

        char peekNext() {
            if (curr + 1 >= src.length()) return '\0'; // reached end
            return src.at(curr + 1);
        }

        void addToken (TokenType type) { addToken(type, nullptr); }

        void addToken (TokenType type, std::any literal) {
            std::string text{src.substr(start, curr - start)};
            tokens.emplace_back(type, std::move(text), std::move(literal), line);
        }

        void readStr() {
            while (peek() != '"' && !isAtEnd()) { // read string until ending double quotes
                if (peek() == '\n') line++;
                advance();
            }

            if (isAtEnd()) { // no terminating double quote
                error(line, "Unterminated string.");
                return;
            }

            // consume the closing double quote
            advance();

            // trim the surring quotes 
            std::string val{src.substr(start + 1, curr - 2 - start)};
            addToken(String, val);
        }

        void readNum() {
            while (isDigit(peek())) advance();

            // see if it's a float
            if (peek() == '.' && isDigit(peekNext())) advance(); // consume decimal point char
            while (isDigit(peek())) advance(); // get rest of num

            addToken(Number, std::stod(std::string{src.substr(start, curr - start)})); // add new num token and convert str to a double 
        }     

        void readId () {
            while (isAlphaNumeric(peek())) advance(); 

            //add ID token
            std::string str = std::string{src.substr(start, curr - start)};
            TokenType type;
            auto match = keywords.find(str);
            if (match == keywords.end()) type = Identifier;
            else type = match->second;

            addToken(type);
        }   

        void tokenScan() {
            char curr = advance();
            switch (curr) {
                // single char token scan
                case '(' : 
                    addToken(OpenPar);
                    break;
                case ')' :
                    addToken(ClosePar);
                    break;
                case '{' :
                    addToken(OpenBrace);
                    break;
                case '}' : 
                    addToken(CloseBrace);
                    break;
                case ',' :
                    addToken(Comma);
                    break;
                case '.' : 
                    addToken(Dot);
                    break;
                case '-' :
                    addToken(Minus);
                    break;
                case '+' :
                    addToken(Plus);
                    break;
                case ';' :
                    addToken(Semicolon);
                    break;
                case '*' :
                    addToken(Star);
                    break;
                case '/' :
                    // if double slash = comment, so consume
                    if (matchMe('/')) { while(peek() != '\n' && !isAtEnd()) advance(); }
                    else addToken(Slash);
                    break;

                // one or two char tokens
                case '!' : 
                    addToken(matchMe('=') ? BangEqual : Bang);
                    break;
                case '=' :
                    addToken(matchMe('=') ? EqualEqual : Equal);
                    break;
                case '<' :
                    addToken(matchMe('=') ? LessEqual : Less);
                    break;
                case '>' :  
                    addToken(matchMe('=') ? GreaterEqual : Greater);
                    break;
                
                // ignoring whitespace... if newline, update line counter
                case ' '  :
                case '\r' :
                case '\t' :
                    break;
                case '\n' :
                    line++; break;

                // string literals 
                case '"' : 
                    readStr(); break;
                
                default: // this is where we will take care of nums and identifiers 
                    if (isDigit(curr)) readNum(); 
                    else if (isAlpha(curr)) readId();
                    else error(line, "Unexpected character."); 
                    break;
            }
        }
};

const std::unordered_map<std::string, TokenType> Scanner::keywords = {
    {"and", TokenType::And},
    {"class", TokenType::Class},
    {"else", TokenType::Else},
    {"false", TokenType::False},
    {"for", TokenType::For},
    {"fun", TokenType::Fun},
    {"if", TokenType::If},
    {"nil", TokenType::Nil},
    {"or", TokenType::Or},
    {"print", TokenType::Print},
    {"return", TokenType::Return},
    {"super", TokenType::Super},
    {"this", TokenType::This},
    {"true", TokenType::True},
    {"var", TokenType::Var},
    {"while", TokenType::While},
};