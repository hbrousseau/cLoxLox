#include "ASTPrint.hpp"

int main(int argc, char* argv[]) {
    std::shared_ptr<Expr> expression = std::make_shared<binaryExpr>(
        std::make_shared<unaryExpr>(
            Token{Minus, "-", nullptr, 1},
            std::make_shared<literalExpr>(123.)
        ),
        Token{Star, "*", nullptr, 1},
        std::make_shared<groupingExpr>(
            std::make_shared<literalExpr>(45.67)));

    std::cout << ASTPrinter{}.print(expression) << std::endl;
}