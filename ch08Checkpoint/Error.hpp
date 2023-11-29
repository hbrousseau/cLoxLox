#pragma once

#include <iostream>
#include <string_view>

#include "RuntimeError.hpp"
#include "Token.hpp"

inline bool hadError = false;
inline bool hadRuntimeError = false; // add in ch07

static void report (int line, std::string_view where, std::string_view message) {
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
    hadError = true;
}

void error (const Token& token, std::string_view message) { // updated this chapter
    if (token.type == Eof) report(token.line, " at end", message);

    else report(token.line, " at '" + token.lexeme + "'", message);
}

void error (int line, std::string_view message) { report(line, "", message); }

// added in ch07
void runtimeError (const RuntimeError& error) {
    std::cerr << error.what() << std::endl << "[line " << error.token.line << "]" << std::endl;
    hadRuntimeError = true;
}