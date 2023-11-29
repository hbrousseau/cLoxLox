#pragma once

#include <any>
#include <map> // for some reason, I had to use this to get the map to work... unordered_map threw linking errors and is probably something with the compiler
#include <memory>
#include <string>
#include <vector>
// #include <unordered_map>

#include "Interpreter.hpp"
#include "LoxCallable.hpp"
#include "LoxFunction.hpp"

// note: I attempted to make this header file without the cpp file, but I was getting a lot of errors. I attempted to use the keyword "inline" 
// but that did not work. I am not sure why, but I will have to look into it later. For now, I am just creating the cpp files where necessary.

class Interpreter;
class LoxFunction;

// class LoxClass : public std::enable_shared_from_this<LoxClass> {
class LoxClass : public LoxCallable, public std::enable_shared_from_this<LoxClass> {
    private: 
        std::string name;
        std::map<std::string, std::shared_ptr<LoxFunction>> methods;
        friend class LoxInstance;

    public:
        //LoxClass (std::string name);
        LoxClass (std::string name, std::map<std::string, std::shared_ptr<LoxFunction>> methods);

        std::string toString();

        std::any call (Interpreter& interpreter, std::vector<std::any> arguments) override;

        int arity() override;

        std::shared_ptr<LoxFunction> findMethod (const std::string& name); 

};