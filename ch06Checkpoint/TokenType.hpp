#pragma once 

#include <string>

enum TokenType {
    // single-chars 
    OpenPar, ClosePar, OpenBrace, CloseBrace,
    Comma, Dot, Minus, Plus, Semicolon, Slash, Star,

    // one or two char tokens
    Bang, BangEqual,
    Equal, EqualEqual,
    Greater, GreaterEqual,
    Less, LessEqual,

    // literals
    Identifier, String, Number,

    // keywords 
    And, Class, Else, False, Fun, For, If, Nil, Or,
    Print, Return, Super, This, True, Var, While,

    // end of file
    Eof
};

// making the enum tokens strings for ease of use later on
inline std::string tokensToString(TokenType type) {
    static const std::string strings[] = {
    "OpenPar", "ClosePar", "OpenBrace", "CloseBrace",
    "Comma", "Dot", "Minus", "Plus", "Semicolon", "Slash", "Star",
    "Bang", "BangEqual",
    "Equal", "EqualEqual",
    "Greater", "GreaterEqual",
    "Less", "LessEqual",
    "Identifier", "String", "Number",
    "And", "Class", "Else", "False", "Fun", "For", "If", "Nil", "Or",
    "Print", "Return", "Super", "This", "True", "Var", "While",
    "Eof"
    };

    return strings[static_cast<int>(type)]; 
}