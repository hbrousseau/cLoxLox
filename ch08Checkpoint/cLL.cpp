#include <cstring> // for std::strerror
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "ASTPrint.hpp"
#include "Error.hpp"
#include "Interpreter.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"

Interpreter interpreter{}; // added in ch07

std::string read(std::string_view fileName) {
    std::ifstream file{fileName.data()};
    if (!file) {
        std::cerr << "Failed opening file " << fileName << ": " << std::strerror(errno) << std::endl;
        std::exit(64);
    }

    std::string readMe((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return readMe;
}

// Function to run the interpreter on a provided source code string
void run (std::string_view src) {
    Scanner scanner {src}; // create a scanner for the source code
    std::vector<Token> tokens = scanner.scanTokens(); // get tokens based on source

    Parser parser{tokens};
    // std::shared_ptr<Expr> expression = parser.parse(); // since ch08
    std::vector<std::shared_ptr<Stmt>> statements = parser.parse();

    // stop if syntx error
    if (hadError) return;
    // std::cout << ASTPrinter{}.print(expression) << std::endl; ... deleted in ch07
    //interpreter.interpret(expression); // added in ch07... changed in ch08
    interpreter.interpret(statements);
}

// Function to run the interpreter on a source code file
void runFile (std::string_view src) {
    std::string info = read(src); // read src and store its contents inside info
    run(info); // execute the interpreter on the source code

    if (hadError) std::exit(65); // If there was an error, exit with an error code
    if (hadRuntimeError) std::exit(70); // added in ch07
}

// Function to run the interpreter in a REPL
void runPrompt() {
    for (;;) {
        std::cout << "cLL> ";
        std::string line;
        if (!std::getline(std::cin, line)) break; // if nothing left, break
        run(line);
        hadError = false;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: Lox [script]" << std::endl;
        exit(64);
    }

    else if (argc == 2) runFile(argv[1]); // Execute the interpreter on the provided script file
    else runPrompt(); // Run the interactive prompt if no script is provided
}