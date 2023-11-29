// updated in ch10

#pragma once

#include <iostream>
#include <string_view>

#include "RuntimeError.hpp"
#include "Token.hpp"

inline bool hadError = false;
inline bool hadRuntimeError = false; // add in ch07

// Declare functions as inline to avoid multiple definitions
inline void report(int line, std::string_view where, std::string_view message) {
    std::cerr << "[line " << line << "] Error" << where << ": " << message << "\n";
    hadError = true;
}

inline void error(const Token& token, std::string_view message) { // updated this chapter
    if (token.type == Eof) report(token.line, " at end", message);
    else report(token.line, " at '" + token.lexeme + "'", message);
}

inline void error(int line, std::string_view message) {
    report(line, "", message);
}

// added in ch07
inline void runtimeError(const RuntimeError& error) {
    std::cerr << error.what() << "\n" << "[line " << error.token.line << "]" << "\n";
    hadRuntimeError = true;
}
