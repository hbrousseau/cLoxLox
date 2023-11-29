#include "ASTPrint.hpp"

int main(int argc, char* argv[]) {
    std::shared_ptr<Expr> expression = std::make_shared<Binary>(
        std::make_shared<Unary>(
            Token{Minus, "-", nullptr, 1},
            std::make_shared<Literal>(123.)
        ),
        Token{Star, "*", nullptr, 1},
        std::make_shared<Grouping>(
            std::make_shared<Literal>(45.67)));

    std::cout << AstPrinter{}.print(expression) << "\n";
}