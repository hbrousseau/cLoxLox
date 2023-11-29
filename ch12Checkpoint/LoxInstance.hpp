#pragma once

#include <any>
#include <map> // same reason as in LoxClass.hpp
#include <memory>
#include <string>
// #include <unordered_map>    

class LoxClass;
class Token;

class LoxInstance: public std::enable_shared_from_this<LoxInstance> {
    private: 
        std::shared_ptr<LoxClass> klass;
        std::map<std::string, std::any> fields;

    public: 
        LoxInstance (std::shared_ptr<LoxClass> klass);

        std::any get (const Token& name);

        void set (const Token& name, std::any value);

        std::string toString();
};