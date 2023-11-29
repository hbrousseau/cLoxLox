#pragma once

#include <iostream>
#include <string_view>

#include "Token.hpp"

inline bool hadError = false;

static void report (int line, std::string_view where, std::string_view message) {
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
    hadError = true;
}

void error (const Token& token, std::string_view message) { // updated this chapter
    if (token.type == Eof) report(token.line, " at end", message);

    else report(token.line, " at '" + token.lexeme + "'", message);
}

void error (int line, std::string_view message) { report(line, "", message); }