#include "LoxClass.hpp" // my compiler didn't like this to be after the other includes :(
 
#include <utility> 

// LoxClass::LoxClass(std::string name) : name{std::move(name)} {}

// LoxClass::LoxClass (std::string name, std::map<std::string, std::shared_ptr<LoxFunction>> methods)
//  : name{std::move(name)}, methods{std::move(methods)} {} // updated in ch13

LoxClass::LoxClass(std::string name, std::shared_ptr<LoxClass> superclass, std::map<std::string, std::shared_ptr<LoxFunction>> methods)
  : superclass{superclass}, name{std::move(name)}, methods{std::move(methods)} {} // added in ch13

// updated in ch13
std::shared_ptr<LoxFunction> LoxClass::findMethod (const std::string& name) {
  auto method = methods.find(name);
  if (method != methods.end()) return method->second;
  if (superclass != nullptr) return superclass->findMethod(name); // added in ch13
  return nullptr;
}

std::string LoxClass::toString() { return name; }

std::any LoxClass::call(Interpreter& interpreter, std::vector<std::any> arguments) {
  auto instance = std::make_shared<LoxInstance>(shared_from_this());
  std::shared_ptr<LoxFunction> initializer = findMethod("init");
  if (initializer != nullptr) initializer->bind(instance)->call(interpreter, std::move(arguments));

  return instance;
}

int LoxClass::arity() {
  // return 0;
  std::shared_ptr<LoxFunction> initializer = findMethod("init");
  if (initializer == nullptr) return 0;
  return initializer->arity();
}