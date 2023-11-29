#include "LoxInstance.hpp"
#include "Error.hpp"

#include <utility>

LoxInstance::LoxInstance(std::shared_ptr<LoxClass> klass) : klass{std::move(klass)} {}

std::any LoxInstance::get(const Token& name) {
  auto field = fields.find(name.lexeme);
    if (field != fields.end()) return field->second;

    std::shared_ptr<LoxFunction> method = klass->findMethod(name.lexeme);
    // if (method != nullptr) return method;
    if (method != nullptr) return method->bind(shared_from_this());

    throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set (const Token& name, std::any value) { fields[name.lexeme] = std::move(value); }

std::string LoxInstance::toString() { return klass->name + " instance"; }