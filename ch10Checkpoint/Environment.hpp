#pragma once

#include <any> 
#include <functional>
#include <memory>
#include <string>
#include <unordered_map> 
#include <utility>

#include "Error.hpp"
#include "Token.hpp"

class Environment: public std::__enable_shared_from_this<Environment> {
    private:
        friend class Interpreter;
        std::unordered_map<std::string, std::any> values;
        std::shared_ptr<Environment> enclosing;

    public:
        Environment() : enclosing{nullptr} {}

        Environment (std::shared_ptr<Environment> enclosing) : enclosing{std::move(enclosing)} {}

        // var definition that binds a name to a val
        void define (const std::string& name, std::any val) { values[name] = std::move(val); } 

        // a way to look up var once defined
        std::any get (const Token& name) {
            auto grabMe = values.find(name.lexeme);
            if (grabMe != values.end()) return grabMe->second;

            if (enclosing != nullptr) return enclosing->get(name);

            throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
        }

        void assign (const Token& name, std::any value) {
            auto assignMe = values.find(name.lexeme);
            if (assignMe != values.end()) {
                assignMe->second = std::move(value);
                return;
            }

            if (enclosing != nullptr) {
                enclosing->assign(name, std::move(value));
                return;
            }

            throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
        }
};